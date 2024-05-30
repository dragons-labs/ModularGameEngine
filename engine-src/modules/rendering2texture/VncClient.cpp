/*
Copyright (c) 2015-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

Based on:
	keycode conversion code
		→ OIS zlib/libpng license code
			Copyright (c) 2005-2010 Phillip Castaneda (pjcast -- www.wreckedgames.com)
			see ADDITIONAL_COPYRIGHT/OIS.txt file
		→ X.org X11 license code
			Copyright 1987, 1994, 1998  The Open Group
			Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts
			see ADDITIONAL_COPYRIGHT/X11.txt file
*/

#include "modules/rendering2texture/VncClient.h"

#include "ConfigParser.h"

#include "gui/GuiSystem.h"
#include "input/InputSystem.h"
#include "rendering/RenderingSystem.h"
#include "data/utils/OgreSceneObjectInfo.h"

#include <OgreEntity.h>

#if defined MGE_DEBUG_LEVEL and MGE_DEBUG_LEVEL > 1
#define DEBUG2_LOG(a) LOG_XDEBUG(a)
#else
#define DEBUG2_LOG(a)
#endif

MGE::VNCclient::~VNCclient() {
	LOG_INFO("destroy VNCclient");
	
	if (screenBuf) {
		free(screenBuf);
	}
	Ogre::Root::getSingletonPtr()->removeFrameListener(this);
}

MGE::VNCclient::VNCclient(
	const std::string_view& _objectName, const std::string& host, int display,
	MGE::InteractiveTexture::Mode _mode,
	Ogre::SceneManager* _scnMgr, bool _isInteractive, bool _isNotMovable, bool _disableAlpha, Ogre::MovableObject* _ogreObject
) :
	MGE::InteractiveTexture("VNCclient", _objectName, _mode, _scnMgr, _isNotMovable, _disableAlpha, _ogreObject), 
	MGE::Unloadable(200),
	MGE::AsioSyn(),
	screenBuf(nullptr), haveInput(false), haveCursor(false)
{
	LOG_INFO("Create VNC texture client");
	uint16_t xSize, ySize;
	
	try {
		char msgBuf[32];
		
		LOG_INFO("VNCclient: open TCP connection");
		asioInit(host, Ogre::StringConverter::toString(5900 + display));
		vncRequestMode  = 3;
		
		LOG_INFO("VNCclient: handshake");
		readData(msgBuf, 12);
		std::string_view msgStr(msgBuf);
		if (msgStr != "RFB 003.003\n" && msgStr != "RFB 003.007\n" && msgStr != "RFB 003.008\n") {
			throw std::logic_error("Wrong RFB version: " + msgStr);
		}
		
		sendData("RFB 003.003\n", 12);
		
		readData(msgBuf, 4);
		uint32_t authMode = ntohl(*reinterpret_cast<uint32_t*>(msgBuf));
		if (authMode == 2) {
			throw std::logic_error("VNC Authentication not supported");
		} else if (authMode != 1) {
			throw std::logic_error("VNC Authentication error");
		}
		
		
		LOG_INFO("VNCclient: init");
		msgBuf[0] = 1;                       // shared-flag
		sendData(msgBuf, 1);
		
		readData(msgBuf, 24);
		xSize = ntohs(*reinterpret_cast<uint16_t*>(msgBuf));
		ySize = ntohs(*reinterpret_cast<uint16_t*>(msgBuf+2));
		dropData( ntohl(*reinterpret_cast<uint32_t*>(msgBuf+20)) );
		
		
		LOG_INFO("VNCclient: Set Pixel Format");
		msgBuf[ 0] =   0;                     // message type
		msgBuf[ 4] =  32;                     // bits
		msgBuf[ 5] =  24;                     // depth
		msgBuf[ 6] =   1;                     // big-endian (network byte order)
		msgBuf[ 7] =   1;                     // true color
		msgBuf[ 8] =   0;  msgBuf[ 9] = 0xff; // red max
		msgBuf[10] =   0;  msgBuf[11] = 0xff; // green max
		msgBuf[12] =   0;  msgBuf[13] = 0xff; // blue max
		msgBuf[14] =   8;                     // red shift
		msgBuf[15] =  16;                     // green shift
		msgBuf[16] =  24;                     // blue shift
		sendData(msgBuf, 20);
		
		
		LOG_INFO("VNCclient: Set Encodings");
		msgBuf[ 0] =   2;                    // message type
		msgBuf[ 2] =   0;  msgBuf[ 3] =   1; // number of encodings
		msgBuf[ 4] =   0;  msgBuf[ 5] =   0;  msgBuf[ 6] =   0;  msgBuf[ 7] =   0;  // raw
		//msgBuf[ 8] =   0;  msgBuf[ 9] =   0;  msgBuf[10] =   0;  msgBuf[11] =   1;  // copy rectangle
		sendData(msgBuf, 8);
		
	} catch (std::exception& e) {
		LOG_WARNING("VNC Connection host=\"" + host + ":" + Ogre::StringConverter::toString(display) + "\" node=\"" + getObjectName() + "\" error: " + e.what());
		return;
	}
	
	LOG_INFO("VNCclient: prepare texture, material and screen buffer");
	createTexture(xSize, ySize, _isInteractive);
	screenLineSize = xSize * 4;
	screenBufSize  = screenLineSize * ySize;
	screenBuf      = reinterpret_cast<char*>(malloc(screenBufSize));
	
	LOG_INFO("VNCclient: starting senders and listeners");
	Ogre::Root::getSingletonPtr()->addFrameListener(this);
	
	sleep(1);
	asioIO.restart();
	vncRequestMode  = 0;
	networkListener = new std::thread(std::bind(&MGE::VNCclient::rfbListener, this));
	networkSender   = new std::thread(std::bind(&MGE::VNCclient::rfbSender, this));
}

/**
@page XMLSyntax_MapAndSceneConfig

@subsection XMLNode_VNCclient \<VNCclient\>

@c \<VNCclient\> is used for creating VNC client with rendering to Ogre texture or CEGUI window.

Attributes independent of mode (Ogre vs CEGUI):
	- @c host   - host address to connect (as node value string)
	- @c screen - screen number connect (as node value integer), default 1, used to calcluate port number
	.
Attributes for CEGUI texture and window target:
	- @c windowName - set base name for @ref MGE::InteractiveTexture,
	  used as part of texture and CEGUI image name
	- @c putOnWindow - name of CEGUI window to set its "Image" property to image with subview texture
	.
Attributes for Ogre texture and 3D world object target:
	- @c nodeName   - set base name for @ref MGE::InteractiveTexture,
	  used as part of texture and Ogre material/datablock name,
	  used ALSO as name of Ogre::MovableObject and its parent Ogre::SceneNode (to find 3D object to put texture on it)
	- @c disableAlpha - when true disable using alpha for transparency (@ref XML_Bool, default: true)
*/

MGE::VNCclient* MGE::VNCclient::create(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context) {
	std::string host         = xmlNode.attribute("host").as_string("localhost");
	int         screen       = xmlNode.attribute("screen").as_int(1);
	bool        disableAlpha = xmlNode.attribute("disableAlpha").as_bool(true);
	
	pugi::xml_attribute xmlAttrib;
	if ( (xmlAttrib = xmlNode.attribute("windowName")) ) {
		auto iTexObj = new MGE::VNCclient(xmlAttrib.as_string(), host, screen, MGE::InteractiveTexture::OnGUIWindow, context->scnMgr);
		iTexObj->putOnGUIWindow( xmlNode.attribute("putOnWindow").as_string() );
		return iTexObj;
	} else if ( (xmlAttrib = xmlNode.attribute("nodeName")) ) {
		return new MGE::VNCclient(xmlAttrib.as_string(), host, screen, MGE::InteractiveTexture::OnOgreObject, context->scnMgr, true, false, disableAlpha);
	} else {
		LOG_WARNING("Can't determined MGE::InteractiveTexture::Mode for VNCclient");
		return nullptr;
	}
}

MGE_CONFIG_PARSER_MODULE_FOR_XMLTAG(VNCclient) {
	return MGE::VNCclient::create(xmlNode, context);
}

void MGE::VNCclient::parseServerMessage(const boost::system::error_code& ec, std::size_t transferredBytes, char* messageBuffor) {
	DEBUG2_LOG("VNCclient: parseServerMessage");
	switch(messageBuffor[0]) {
		case 0: {
			readData(messageBuffor, 3);
			uint16_t numOfRects = ntohs(*reinterpret_cast<uint16_t*>(messageBuffor+1));
			
			for (; numOfRects>0; --numOfRects) {
				readData(messageBuffor, 12);
				
				uint16_t* uint16Buf = reinterpret_cast<uint16_t*>(messageBuffor);
				uint16_t x   = htons(uint16Buf[0]);
				uint16_t y   = htons(uint16Buf[1]);
				uint16_t w   = htons(uint16Buf[2]);
				uint16_t h   = htons(uint16Buf[3]);
				uint32_t enc = ntohl(*reinterpret_cast<uint32_t*>(messageBuffor+8));
				
				DEBUG2_LOG("  numOfRects=" << numOfRects << " x=" << x << " y=" << y << " w=" << w << " h=" << h << " enc=" << enc);
				
				std::size_t xOffset    = x * 4;
				std::size_t lineSize   = w * 4;
				std::size_t yOffset    = y * screenLineSize;
				std::size_t numOfLines = h;
				
				if (enc == 0) { // raw
					for (; numOfLines>0; --numOfLines) {
						DEBUG2_LOG("  numOfLines=" << numOfLines << "  offset=" << xOffset + yOffset);
						readData(screenBuf + xOffset + yOffset, lineSize);
						yOffset += screenLineSize;
					}
				} else {
					throw std::logic_error("unsupported FramebufferUpdate encoding: " + Ogre::StringConverter::toString(enc));
				}
			}
			
			screenBufNeedRedraw = true;
			break;
		}
		case 1:
			readData(messageBuffor, 5);
			dropData( 6 * ntohs(*reinterpret_cast<uint16_t*>(messageBuffor+3)) );
			break;
		case 2:
			// bell
			break;
		case 3:
			readData(messageBuffor, 5);
			dropData( ntohs(*reinterpret_cast<uint32_t*>(messageBuffor+3)) );
			break;
		default:
			throw std::logic_error("unsupported server message type: " + Ogre::StringConverter::toString(messageBuffor[0]));
	}
}

void MGE::VNCclient::sendFramebufferUpdateRequest(char incremental, bool doPool) {
	char msgBuf[16];
	
	DEBUG2_LOG("VNCclient: sendFramebufferUpdateRequest");
	msgBuf[ 0] =   3;                                             // message type
	msgBuf[ 1] =   incremental;                                   // incremental
	msgBuf[ 2] =   0;  msgBuf[ 3] =   0;                          // x
	msgBuf[ 4] =   0;  msgBuf[ 5] =   0;                          // y
	uint16_t* uint16Buf = reinterpret_cast<uint16_t*>(msgBuf+6);
	uint16Buf[0] = htons(renderTexture->getWidth());              // width
	uint16Buf[1] = htons(renderTexture->getHeight());             // height
	
	sendData(msgBuf, 10, 2, doPool);
}

[[ noreturn ]] /* infinite loop */ void MGE::VNCclient::rfbListener() {
	char msgBuf[16];
	while(true) {
		try {
			boost::asio::async_read(
				*asioSocket, boost::asio::buffer(msgBuf, 1),
				boost::asio::transfer_exactly(1),
				std::bind(&MGE::VNCclient::parseServerMessage, this, std::placeholders::_1, std::placeholders::_2, msgBuf)
			);
			asioIO.run();
			asioIO.restart();
		} catch (std::exception& e) {
			LOG_WARNING("VNC listener error: " << e.what());
			
			vncRequestMode = -1;
			for (int i=0 ; i<3; ++i) {
				std::this_thread::sleep_for(std::chrono::milliseconds(500));
				boost::asio::socket_base::bytes_readable bytesReadable(true);
				asioSocket->io_control(bytesReadable);
				dropData( bytesReadable.get() );
			}
			vncRequestMode = 1;
		}
	}
}

[[ noreturn ]] /* infinite loop */ void MGE::VNCclient::rfbSender() {
	while(true) {
		std::this_thread::sleep_for(std::chrono::milliseconds(30));
		try {
			if (vncRequestMode != -1) {
				sendFramebufferUpdateRequest(vncRequestMode, false);
				vncRequestMode = 1;
			}
		} catch (std::exception& e) {
			LOG_WARNING("VNC sender error: " << e.what());
			vncRequestMode = 1;
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
	}
}



void MGE::VNCclient::sendMouseEvent(const OIS::MouseEvent& arg, uint8_t buttonMask) {
	char msgBuf[8];
	
	if (arg.state.buttonDown(OIS::MB_Left)) {
		buttonMask |= 1 << 0;
	}
	if (arg.state.buttonDown(OIS::MB_Middle)) {
		buttonMask |= 1 << 1;
	}
	if (arg.state.buttonDown(OIS::MB_Right)) {
		buttonMask |= 1 << 2;
	}
	
	DEBUG2_LOG("VNCclient: sendMouseEvent");
	msgBuf[ 0] =   5;                                                         // message type
	msgBuf[ 1] =   buttonMask;                                                // button mask
	uint16_t* uint16Buf = reinterpret_cast<uint16_t*>(msgBuf+2);
	uint16Buf[0] = htons(renderTexture->getWidth() * lastMouseTexturePos.x);  // x
	uint16Buf[1] = htons(renderTexture->getHeight() * lastMouseTexturePos.y); // y
	sendData(msgBuf, 6, 2, false);
}

bool MGE::VNCclient::mousePressed(const Ogre::Vector2& mouseTexturePos, OIS::MouseButtonID buttonID, const OIS::MouseEvent& arg) {
	lastMouseTexturePos = mouseTexturePos;
	
	if (haveInput) {
		sendMouseEvent(arg);
	} else {
		MGE::GUISystem::getPtr()->setMouseVisible(false);
		haveInput = true;
		haveCursor = true;
	}
	
	return true;
}

bool MGE::VNCclient::mouseMoved(const Ogre::Vector2& mousePos, const OIS::MouseEvent& arg) {
	uint8_t buttonMask = 0;
	
	if (arg.state.Z.rel > 0) {
		buttonMask |= 1 << 3;
	} else if (arg.state.Z.rel < 0) {
		buttonMask |= 1 << 4;
	}
	
	if (arg.state.X.rel || arg.state.Y.rel) {
		std::pair<bool, Ogre::Vector2> res = textureHitTest(mousePos);
		if (res.first) {
			if (!haveCursor) {
				MGE::GUISystem::getPtr()->setMouseVisible(false);
				haveCursor = true;
			}
			lastMouseTexturePos = res.second;
			sendMouseEvent(arg, buttonMask);
		} else {
			if (haveCursor) {
				MGE::GUISystem::getPtr()->setMouseVisible(true);
				haveCursor = false;
			}
			return false;
		}
	}
	return true;
}

bool MGE::VNCclient::mouseReleased( const Ogre::Vector2& mousePos, OIS::MouseButtonID buttonID, const OIS::MouseEvent& arg ) {
	std::pair<bool, Ogre::Vector2> res = textureHitTest(mousePos);
	if (res.first) {
		sendMouseEvent(arg);
		return true;
	} else {
		return false;
	}
}

void MGE::VNCclient::sendKeyEvent(const OIS::KeyEvent& arg, bool isDown) {
	LOG_DEBUG("VNCclient: sendKeyEvent: isDown=" << isDown << " arg.key=" << arg.key << " arg.text=" << arg.text);
	
	char msgBuf[16];
	msgBuf[ 0] =   4;                                            // message type
	msgBuf[ 1] =   isDown;                                       // down flag
	uint32_t* uint32Buf = reinterpret_cast<uint32_t*>(msgBuf+4); // X11 keysym
	
	*uint32Buf = htonl(getX11KeySym(arg.key, arg.text, isDown));
	
	sendData(msgBuf, 8, 2, false);
}

bool MGE::VNCclient::keyPressed(const OIS::KeyEvent& arg) {
	sendKeyEvent(arg, 1);
	return true;
}

bool MGE::VNCclient::keyReleased(const OIS::KeyEvent& arg) {
	sendKeyEvent(arg, 0);
	return true;
}

bool MGE::VNCclient::lostInput(MGE::InteractiveTexture* toTexture, bool toGUI) {
	MGE::GUISystem::getPtr()->setMouseVisible(true);
	haveInput = false;
	haveCursor = false;
	return true;
}

#include <OgreTextureGpuManager.h>
#include <OgreTextureBox.h>
#include <OgreStagingTexture.h>
bool MGE::VNCclient::frameStarted(const Ogre::FrameEvent& evt) {
	if (!isPaused && screenBufNeedRedraw) {
		fillTexture(reinterpret_cast<Ogre::uint8*>(screenBuf));
		screenBufNeedRedraw = false;
	}
	return true;
}


#ifdef TARGET_SYSTEM_IS_UNIX
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#endif

uint32_t lastTxt[0xff]{};

uint32_t MGE::VNCclient::getX11KeySym(OIS::KeyCode key, uint32_t text, bool isDown) {
	#ifdef TARGET_SYSTEM_IS_UNIX
	if (key == OIS::KC_UNASSIGNED) {
		return NoSymbol;
	}
	#endif
	
	if (isDown) {
		lastTxt[key] = text;
	} else {
		text = lastTxt[key];
		lastTxt[key] = 0;
	}
	
	if (text > 31 && text < 256) { // use ASCII + LATIN-1 ("Western European") == ISO 8859-1
		LOG_DEBUG("Convert " << std::hex << key << " as ASCII " << std::hex << text);
		return text; 
	} else if (text > 255) { // use Unicode for "modern systems"
		/*
		 * can't use this for non-ASCII subset of ISO8859-1 due to bug in XmbLookupString & Xutf8LookupString
		 * (return LATIN-1 string instead of UTF-8 string for 0x1000080 .. 0x10000ff); XLookupString works OK
		 */
		LOG_DEBUG("Convert " << std::hex << key << " as Unicode " << std::hex << text);
		return 0x1000000 + text;
	} else {
		#ifdef TARGET_SYSTEM_IS_UNIX
			auto keyNameID = MGE::InputSystem::getPtr()->getKeyboard()->getAsString(key);
			KeySym keySym = XStringToKeysym(keyNameID.c_str());
			LOG_DEBUG("Convert " << std::hex << key << " (text: " << text << ")" << " to " << std::hex << keySym << "(" << keyNameID << ")");
			return keySym;
		#else
			switch(key) {
				case OIS::KC_ESCAPE:
					return 0xff1b;
				case OIS::KC_F1:
					return 0xffbe;
				case OIS::KC_F2:
					return 0xffbf;
				case OIS::KC_F3:
					return 0xffc;
				case OIS::KC_F4:
					return 0xffc1;
				case OIS::KC_F5:
					return 0xffc2;
				case OIS::KC_F6:
					return 0xffc3;
				case OIS::KC_F7:
					return 0xffc4;
				case OIS::KC_F8:
					return 0xffc5;
				case OIS::KC_F9:
					return 0xffc6;
				case OIS::KC_F10:
					return 0xffc7;
				case OIS::KC_F11:
					return 0xffc8;
				case OIS::KC_F12:
					return 0xffc9;
				
				case OIS::KC_SYSRQ:
					return 0xff61; //Print
				case OIS::KC_SCROLL:
					return 0xff14;
				case OIS::KC_PAUSE:
					return 0xff13;
				
				case OIS::KC_INSERT:
					return 0xff63;
				case OIS::KC_DELETE:
					return 0xffff;
				case OIS::KC_HOME:
					return 0xff50;
				case OIS::KC_END:
					return 0xff57;
				case OIS::KC_PGUP:
					return 0xff55;
				case OIS::KC_PGDOWN:
					return 0xff56;
					
				case OIS::KC_UP:
					return 0xff52;
				case OIS::KC_DOWN:
					return 0xff54;
				case OIS::KC_LEFT:
					return 0xff51;
				case OIS::KC_RIGHT:
					return 0xff53;
				
				case OIS::KC_TAB:
					return 0xff09;
				case OIS::KC_CAPITAL:
					return 0xffe5;
				case OIS::KC_LSHIFT:
					return 0xffe1;
				case OIS::KC_LCONTROL:
					return 0xffe3;
				case OIS::KC_LWIN:
					return 0xffeb;
				case OIS::KC_LMENU:
					return 0xffe9;
				
				case OIS::KC_BACK:
					return 0xff08;
				case OIS::KC_RETURN:
					return 0xff0d;
				case OIS::KC_RSHIFT:
					return 0xffe2;
				case OIS::KC_RCONTROL:
					return 0xffe4;
				case OIS::KC_RWIN:
					return 0xffec;
				case 0x54:
					return 0xffe3;
				default:
					return 0;
			}
		#endif
	}
}
