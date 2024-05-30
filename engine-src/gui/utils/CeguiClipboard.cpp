/*
Copyright (c) 2017-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>

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

Inspirited by: xsel (https://vergenet.net/~conrad/software/xsel/) code.
*/

#include "gui/utils/CeguiClipboard.h"

#include "LogSystem.h"
#include "StringUtils.h"

#if defined MGE_DEBUG_LEVEL and MGE_DEBUG_LEVEL > 1
#define DEBUG2_LOG(a) LOG_XDEBUG(a)
#else
#define DEBUG2_LOG(a)
#endif

#include <OgreWindow.h>
#include <string>

void MGE::CeguiNativeClipboard::sendToClipboard(const CEGUI::String& mimeType, void* buffer, size_t size) {
	if (mimeType != "text/plain") {
		LOG_DEBUG("non text ... ignoring mimeType=" << mimeType);
		return;
	}
	
	_sendToClipboard(std::string_view(static_cast<const char*>(buffer), size));
}

void MGE::CeguiNativeClipboard::retrieveFromClipboard(CEGUI::String& mimeType, void*& buffer, size_t& size) {
	auto ret = _retrieveFromClipboard();
	mimeType = "text/plain";
	buffer   = static_cast<void*>(const_cast<char*>(ret.data()));
	size     = ret.length();
}

#if defined( TARGET_SYSTEM_IS_UNIX )
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <thread>
#include <condition_variable>
#include <mutex>

struct MGE::CeguiNativeClipboard::PrivData {
	/// X11 window id
	Window    xWindow;
	
	/// X11 display connection
	Display*  xDisplay;
	
	/// String object with strData pointing to X11 buffor or NULL
	std::string_view        outContent;
	bool                    outContentValid;
	
	/// String object with own buffer for clipboard data sending via X11
	std::string             inContent;
	
	/// function used as thread for sending data via X11
	void eventThread();
	
	/// sending data via X11 boost thread pointer
	std::thread*             eventThreadPtr;
	std::mutex               eventThreadMutex;
	std::condition_variable  eventThreadCondition;
	
	/// X11 atoms
	Atom bufforTypeAtom, utf8StringAtom, propAtom, targetQueryAtom;
	
	/// destructor
	~PrivData() {
		LOG_INFO("destroy CeguiNativeClipboard for X11 PrivData");
		
		LOG_INFO("shutdown eventThread");
		XDestroyWindow(xDisplay, xWindow);
		XSync(xDisplay, 0);
		LOG_DEBUG("");
		eventThreadPtr->join();
		LOG_DEBUG("");
		delete eventThreadPtr;
		
		LOG_INFO("free X11 buffor");
		if (outContent.length())
			XFree( static_cast<void*>(const_cast<char*>(outContent.data())) );
		
		LOG_INFO("close X11 display");
		XCloseDisplay(xDisplay);
		
		LOG_INFO("destroy CeguiNativeClipboard for X11 PrivData ... done");
	}
};

MGE::CeguiNativeClipboard::CeguiNativeClipboard(Ogre::Window* renderWindow) {
	LOG_INFO("CeguiNativeClipboard for X11");
	
	// create priv data structure
	privData = new PrivData();
	
	// get X11 window (aka unsigned long)
	privData->xWindow = 0;
	renderWindow->getCustomAttribute("WINDOW",  &(privData->xWindow));
	
	// get X11 Display pointer
	privData->xDisplay = nullptr;
	renderWindow->getCustomAttribute("DISPLAY", &(privData->xDisplay));
	
	LOG_INFO("get renderWindow: " << renderWindow << " XWINDOW: " << privData->xWindow << " and XDISPLAY: " << privData->xDisplay);
	
	// open new connection to display and create new window
	privData->xDisplay = XOpenDisplay(0);
	privData->xWindow = XCreateSimpleWindow(privData->xDisplay, privData->xWindow, 0,0, 1,1, 0, 0, 0);
	
	LOG_INFO("using new XWINDOW: " << privData->xWindow << " and XDISPLAY: " << privData->xDisplay);
	
	// prepare atoms
	privData->bufforTypeAtom  = XInternAtom(privData->xDisplay, "PRIMARY", False); // for ctrl+c / ctrl+v buffer use "CLIPBOARD" insted of "PRIMARY"
	privData->utf8StringAtom  = XInternAtom(privData->xDisplay, "UTF8_STRING", False);
	privData->propAtom        = XInternAtom(privData->xDisplay, "XSEL_DATA", False);
	privData->targetQueryAtom = XInternAtom(privData->xDisplay, "TARGETS", False);
	
	// set event mask and start event listener
	XSelectInput(privData->xDisplay, privData->xWindow, StructureNotifyMask);
	privData->eventThreadPtr = new std::thread(std::bind(&MGE::CeguiNativeClipboard::PrivData::eventThread, privData));
}

void MGE::CeguiNativeClipboard::PrivData::eventThread() {
	LOG_INFO("start eventThread ... thread::id==" << std::this_thread::get_id());
	XEvent event;
	while(1) {
		// wait for event and recive first event in queue
		XNextEvent (xDisplay, &event);
		
		// process event in locked section
		eventThreadMutex.lock();
		DEBUG2_LOG("event.type: " << event.type);
		switch (event.type) {
			// get new selection buffer (input data from X11)
			case SelectionNotify:
				if (outContentValid) {
					LOG_DEBUG("get SelectionNotify when outContent is valid :-/");
				} else if (event.xselection.property) {
					unsigned long tmp, len;
					char* str;
					XGetWindowProperty(
						event.xselection.display, event.xselection.requestor, event.xselection.property,
						0, 65536, False, AnyPropertyType,
						&tmp, reinterpret_cast<int*>(&tmp), &len, &tmp, reinterpret_cast<unsigned char **>(&str)
					);
					XDeleteProperty(event.xselection.display, event.xselection.requestor, event.xselection.property);
					outContent = std::string_view(str, len);
					LOG_DEBUG("recived from X11: " << outContent);
				} else {
					LOG_DEBUG("xselection conversion failed");
				}
				outContentValid = true;
				eventThreadCondition.notify_one();
				break;
			
			// send selection (output data to X11)
			case SelectionRequest: {
				if (inContent.empty())
					break;
				
				XSelectionRequestEvent* xsr = &(event.xselectionrequest);
				DEBUG2_LOG("xselectionrequest: " << xsr->target);
				
				if (xsr->target == targetQueryAtom) {
					// list of supported atom targets
					Atom supportedTargets[2];
					supportedTargets[0] = utf8StringAtom;
					supportedTargets[1] = targetQueryAtom;
					
					XChangeProperty(
						xsr->display, xsr->requestor, xsr->property, XA_ATOM, 32, PropModeReplace,
						reinterpret_cast<unsigned char *>(supportedTargets), static_cast<int>(sizeof(supportedTargets) / sizeof(Atom))
					);
				} else if (xsr->target == utf8StringAtom) {
					XChangeProperty(
						xsr->display, xsr->requestor, xsr->property, utf8StringAtom, 8, PropModeReplace,
						reinterpret_cast<unsigned char *>(const_cast<char*>(inContent.data())), static_cast<int>(inContent.length())
					);
				} else {
					LOG_DEBUG("unsupported xselectionrequest target=" << xsr->target);
					break;
				}
				
				XSelectionEvent ev;
				ev.type = SelectionNotify;
				ev.target = xsr->target;
				ev.property = xsr->property;
				ev.display = xsr->display;
				ev.requestor = xsr->requestor;
				ev.selection = xsr->selection;
				ev.time = xsr->time;
				
				XSendEvent(xsr->display, xsr->requestor, 0, 0, reinterpret_cast<XEvent*>(&ev));
				XFlush(xsr->display);
				break;
			}
			
			case SelectionClear:
				LOG_DEBUG("clear selection ...");
				inContent.clear();
				break;
			
			case DestroyNotify:
				LOG_INFO("stop eventThread ... thread::id==" << std::this_thread::get_id());
				XSync(xDisplay, 1);
				eventThreadMutex.unlock();
				return;
				
			default:
				LOG_DEBUG("unsupported event type=" << event.type);
		}
		eventThreadMutex.unlock();
	}
}

void MGE::CeguiNativeClipboard::_sendToClipboard(const std::string_view str) {
	privData->inContent = str;
	
	if (privData->inContent.empty()) {
		// clear X11 selection
		XSetSelectionOwner( privData->xDisplay, privData->bufforTypeAtom, None, CurrentTime );
	} else {
		// get ownership of X11 selection
		XSetSelectionOwner( privData->xDisplay, privData->bufforTypeAtom, privData->xWindow, CurrentTime );
	}
	XFlush(privData->xDisplay);
}

const std::string_view& MGE::CeguiNativeClipboard::_retrieveFromClipboard() {
	// lock privData
	privData->eventThreadMutex.lock();
	// free old buffer
	privData->outContentValid = false;
	if (privData->outContent.length()) {
		XFree( static_cast<void*>(const_cast<char*>(privData->outContent.data())) );
		privData->outContent = MGE::EMPTY_STRING_VIEW;
	}
	
	// make request about clipboard
	XConvertSelection( privData->xDisplay, privData->bufforTypeAtom, privData->utf8StringAtom, privData->propAtom, privData->xWindow, CurrentTime );
	XFlush( privData->xDisplay );
	
	// unlock privData
	privData->eventThreadMutex.unlock();
	
	// wait for new buffor
	std::unique_lock<std::mutex> privDataLock(privData->eventThreadMutex);
	while(!privData->outContentValid) {
		privData->eventThreadCondition.wait(privDataLock);
	}
	
	// return new buffor string
	return privData->outContent;
}

bool MGE::CeguiNativeClipboard::supported() { return true; }

#elif defined( TARGET_SYSTEM_IS_WINDOWS )

struct MGE::CeguiNativeClipboard::PrivData {};

MGE::CeguiNativeClipboard::CeguiNativeClipboard(Ogre::RenderWindow* renderWindow) {}
void MGE::CeguiNativeClipboard::_sendToClipboard(const String str) {}
const MGE::Utils::String& MGE::CeguiNativeClipboard::_retrieveFromClipboard() { return  MGE::Utils::String::EMPTY; }
bool MGE::CeguiNativeClipboard::supported() { return false; }

#endif

MGE::CeguiNativeClipboard::~CeguiNativeClipboard() {
	delete privData;
}
