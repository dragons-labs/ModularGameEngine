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
	→ public domain code from https://github.com/qwertzui11/cef_osr

Inspired by:
	→ The Chromium Embedded Framework samples code (https://github.com/chromiumembedded/cef/tree/master/tests) (BSD licensed)
*/

#include "modules/rendering2texture/WebBrowser.h"

#include "config-cef.h"

#include "LogSystem.h"
#include "ScriptsSystem.h"

#include "gui/GuiSystem.h"
#include "rendering/RenderingSystem.h"
#include "data/utils/OgreSceneObjectInfo.h"

#include <OgreEntity.h>

#ifdef USE_CEF
#undef LOG_STREAM
#include <cef_app.h>
#include <cef_client.h>
#include <cef_parser.h>
#include <cef_render_handler.h>
#undef LOG_STREAM

#ifndef __DOCUMENTATION_GENERATOR__
class BrowserClient :
	public CefClient, public CefRenderHandler,
	public CefLifeSpanHandler, public CefRequestHandler, public CefJSDialogHandler
{
public:
	MGE::WebBrowser*           parent;
	CefRefPtr<CefBrowser>      browser;
	CefRefPtr<CefBrowserHost>  browserHost;
	
	BrowserClient(MGE::WebBrowser* _parent, const std::string& url) {
		parent = _parent;
		
		// create browser-window
		CefWindowInfo window_info;
		CefBrowserSettings browserSettings;
		window_info.SetAsWindowless(0/*, false*/);
		browser = CefBrowserHost::CreateBrowserSync(
			window_info, this, url, browserSettings, nullptr, nullptr
		);
	}
	
	// CefRenderHandler interface
	virtual void GetViewRect(CefRefPtr<CefBrowser> _browser, CefRect& rect) override {
		rect = CefRect(0, 0, parent->renderTexture->getWidth(), parent->renderTexture->getHeight());
	}
	virtual void OnPaint(CefRefPtr<CefBrowser> _browser, PaintElementType type, const RectList& dirtyRects, const void* buffer, int width, int height) override {
		LOG_DEBUG("OnPaint " << this << " main=" << (this == parent->mainClient) << " dialog=" << (this == parent->dialogClient) << " current=" << (this == parent->currentClient));
		if (parent->currentClient && parent->currentClient != this)
			return;
		
		parent->fillTexture(reinterpret_cast<const Ogre::uint8*>(buffer));
	}
	
	// CefLifeSpanHandler interface
	virtual bool OnBeforePopup(
		CefRefPtr<CefBrowser> _browser, CefRefPtr<CefFrame> frame,
		const CefString& target_url, const CefString& target_frame_name, cef_window_open_disposition_t target_disposition,
		bool user_gesture, const CefPopupFeatures& popupFeatures, CefWindowInfo& windowInfo,
		CefRefPtr<CefClient>& client, CefBrowserSettings& settings, CefRefPtr<CefDictionaryValue>& extra_info, bool* no_javascript_access
	) override {
		_browser->GetMainFrame()->LoadURL(target_url); // force one window mode (no pop-up, no new frame, etc)
		return true;
	}
	
	// helper function for show java script, authentication, etc dialogs as HTML on dialogClient
	// @see processing callback from this form in OnBeforeBrowse
	bool showDialog(int type, const std::string_view& target, const std::string_view& text, const std::string_view& text1 = MGE::EMPTY_STRING_VIEW, const std::string_view& text2 = MGE::EMPTY_STRING_VIEW) {
		if (parent->currentClient == parent->dialogClient) {
			return false;
		}
		
		std::string dialogStr(
			"<html><head>\
				<style>label{display: inline-block; width: 90px; margin-right: .5em; text-align: right;}</style>\
			</head><body style=\"background-color: #cccccc; font-size: 13pt;\">\
				<div style=\"position: absolute; top: 50%; left: 50%; margin-right: -50%; transform: translate(-50%, -50%)\">\
				<div style=\"border: solid; border-radius: 1em; padding: 1em; width: 360px; transform: scale(1.5, 1.5)\">\
					<p>" + text + "</p>\
					<form action=\"" + target + "\" method=\"get\" style=\"text-align: center\">\
					"
		);
		
		bool showCancel = true;
		switch(type) {
			case JSDIALOGTYPE_PROMPT:
				dialogStr.append("<div style=\"margin: .5em;\">\
					<input type=\"text\" name=\"txt\" value=\"" + text1 + "\" />\
					</div>"
				);
				break;
			case JSDIALOGTYPE_CONFIRM:
				break;
			case JSDIALOGTYPE_ALERT:
				showCancel = false;
				break;
			case 100:
				dialogStr.append(
					"<div style=\"margin: .5em;\"><label for=\"login\">Login:</label>\
						<input type=\"text\" name=\"login\" value=\"" + text1 + "\" />\
					</div><div style=\"margin: .5em;\"><label for=\"password\">Password:</label>\
						<input type=\"password\" name=\"password\" value=\"" + text2 + "\" />\
					</div>"
				);
				break;
		}
		
		dialogStr.append("<button type=\"submit\" name=\"val\" value=\"ok\">OK</button>");
		if (showCancel)
			dialogStr.append("&nbsp;&nbsp;& nbsp;&nbsp;<button type=\"submit\" name=\"val\" value=\"cancel\">Cancel</button>");
		
		dialogStr.append("</form></div></div></body></html>");
		
		parent->currentClient = parent->dialogClient;
		parent->currentClient->browser->GetMainFrame()->LoadURL(
			"data:text/html;base64," + CefURIEncode(CefBase64Encode(dialogStr.data(), dialogStr.length()), false).ToString()
		); // can't use parent->loadString() because it must be call on dialogClient not mainClient
		
		return true;
	}
	
	// CefRequestHandler interface
	virtual bool OnBeforeBrowse(
		CefRefPtr<CefBrowser> _browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, bool user_gesture, bool is_redirect
	) override {
		std::string urlScheme, urlPath;
		std::map<std::string, std::string> requestData;
		std::set<std::string> schemes = {"internal", "script"};
		
		LOG_DEBUG("OnBeforeBrowse " << this);
		if (!MGE::WebBrowser::parseUrl(
			request->GetURL().ToString().c_str(), urlScheme, urlPath, requestData, schemes
		)) {
			return false;
		}
		
		parent->currentClient = parent->mainClient;
		parent->currentClient->browser->GetHost()->Invalidate(PET_VIEW);
		
		if (urlScheme == "script") {
			LOG_DEBUG("run script " << urlPath);
			MGE::ScriptsSystem::getPtr()->runObjectWithVoid(
				urlPath.c_str(),
				pybind11::cast(parent, pybind11::return_value_policy::reference),
				requestData
			);
		} else if (urlScheme == "internal") {
			if (urlPath=="jsdialog") {
				bool ret = false;
				if (requestData["val"]=="ok")
					ret = true;
				
				parent->currentClient->jsCallback->Continue(ret, requestData["txt"]);
				parent->currentClient->jsCallback = nullptr;
			} else if (urlPath=="auth") {
				parent->currentClient->authCallback->Continue(requestData["login"], requestData["password"]);
				parent->currentClient->authCallback = nullptr;
			}
		}
		
		return true;
		// false => continue request,
		// true  => cancel request (stay on current page),
		// when in function call browser->GetMainFrame()->LoadURL() with new url return value does not matter (but probably should be true)
	}
	
	// CefRequestHandler interface
	virtual bool GetAuthCredentials(
		CefRefPtr<CefBrowser> _browser, const CefString& origin_url, bool isProxy, const CefString& host, int port,
		const CefString& realm, const CefString& scheme, CefRefPtr<CefAuthCallback> callback
	) override {
		authCallback = callback;
		
		return showDialog(
			100, "internal:auth",
			std::string_view("Authentication request for ") + host.ToString().c_str() + ":<br />" + realm.ToString().c_str()
		);
	}
	
	// CefJSDialogHandler interface
	virtual bool OnJSDialog(
		CefRefPtr<CefBrowser> _browser, const CefString& origin_url,
		/*const CefString& accept_lang,*/ CefJSDialogHandler::JSDialogType dialog_type,
		const CefString& message_text, const CefString& default_prompt_text,
		CefRefPtr<CefJSDialogCallback> callback, bool& suppress_message
	) override {
		jsCallback = callback;
		
		return showDialog(
			dialog_type, "internal:jsdialog",
			message_text.ToString().c_str(),
			default_prompt_text.ToString().c_str()
		);
	}
	
	// CefClient interface
	virtual CefRefPtr<CefRenderHandler> GetRenderHandler() override {
		return this;
	}
	virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override {
		return this;
	}
	virtual CefRefPtr<CefRequestHandler> GetRequestHandler() override {
		return this;
	}
	virtual CefRefPtr<CefJSDialogHandler> GetJSDialogHandler() override {
		return this;
	}
	
	// CefBase interface
	IMPLEMENT_REFCOUNTING(BrowserClient);
	
private:
	CefRefPtr<CefJSDialogCallback> jsCallback;
	CefRefPtr<CefAuthCallback>     authCallback;
};
#endif
#endif



MGE::WebBrowser::WebBrowser(
	const std::string_view& _objectName, int _xSize, int _ySize, const std::string_view& url,
	MGE::InteractiveTexture::Mode _mode,
	Ogre::SceneManager* _scnMgr, bool _isInteractive, bool _isNotMovable, Ogre::MovableObject* _ogreObject
) : 
	MGE::InteractiveTexture("WebBrowser", _objectName, _mode, _scnMgr, _isNotMovable, false, _ogreObject),
	MGE::Unloadable(200)
{
#ifdef USE_CEF
	LOG_INFO("Create WebBrowser texture client");
	
	if (webBrowserObjectCount++ == 0) {
		LOG_INFO("Initialise WebBrowser");
		const char* mainArgsStr[] = {"cef", "--disable-gpu"/*, "--disable-gpu-rasterizer", "--disable-gpu-compositing", "--disable-angle-features", "--gl-null"*/};
		CefMainArgs mainArgs(2, const_cast<char**>(mainArgsStr));
		CefSettings settings;
		
		           settings.no_sandbox                   = true;
		           settings.external_message_pump        = true;
		           settings.multi_threaded_message_loop  = 0;
		           settings.windowless_rendering_enabled = true;
		CefString(&settings.browser_subprocess_path)     = CEF_SUBPROCESS_PATH;
		CefString(&settings.resources_dir_path)          = CEF_RESOURCES_DIR_PATH;
		CefString(&settings.locales_dir_path)            = CEF_LOCALES_DIR_PATH;
		CefString(&settings.log_file)                    = CEF_CONFIG_LOG_PATH;
		           settings.log_severity                 = CEF_CONFIG_LOGSEVERITY;
		CefString(&settings.locale)                      = CEF_CONFIG_LOCALE;
		
		if (! CefInitialize(mainArgs, settings, nullptr, nullptr)) {
			LOG_ERROR("CefInitialize FAIL");
		}
	}
	
	createTexture(_xSize, _ySize, _isInteractive);
	
	currentClient = NULL;
	mainClient    = new BrowserClient(this, parseUrl(url));
	dialogClient  = new BrowserClient(this, "about:blank");
	currentClient = mainClient;
	
	MGE::Engine::handleCrash();
	Ogre::Root::getSingletonPtr()->addFrameListener(this);
#else
	LOG_WARNING("Create **fake** WebBrowser texture client -- build without CEF support.");
#endif
}

MGE::WebBrowser::~WebBrowser() {
	LOG_INFO("destroy WebBrowser");
#ifdef USE_CEF
	Ogre::Root::getSingletonPtr()->removeFrameListener(this);
	
	delete mainClient;
	delete dialogClient;
	/*if (--webBrowserObjectCount == 0) {
		LOG_INFO("destroy CEF");
		CefShutdown();
	}*/
#endif
}

MGE::WebBrowser* MGE::WebBrowser::getBrowser(const std::string_view& name) {
	return static_cast<MGE::WebBrowser*>(
		MGE::InteractiveTextureManager::getPtr()->getTextureListener( name )
	);
}


/**
@page XMLSyntax_MapAndSceneConfig

@subsection XMLNode_WebBrowser \<WebBrowser\>

@c \<WebBrowser\> is used for creating web browser client with rendering to Ogre texture or CEGUI window.

Attributes independent of mode (Ogre vs CEGUI):
	- @c resX - x (horizontal) resolution of render target texture
	- @c resY - y (vertical) resolution of render target texture
	- @c url  - start page to show, default empty page (about:blank)
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
*/

MGE::WebBrowser* MGE::WebBrowser::create(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context) {
	int           resX = xmlNode.attribute("resX").as_int();
	int           resY = xmlNode.attribute("resY").as_int();
	std::string   url  = xmlNode.attribute("url").as_string("about:blank");
	
	pugi::xml_attribute xmlAttrib;
	if ( (xmlAttrib = xmlNode.attribute("windowName")) ) {
		auto iTexObj = new MGE::WebBrowser(xmlAttrib.as_string(), resX, resY, url, MGE::InteractiveTexture::OnGUIWindow, context->scnMgr);
		iTexObj->putOnGUIWindow( xmlNode.attribute("putOnWindow").as_string() );
		return iTexObj;
	} else if ( (xmlAttrib = xmlNode.attribute("nodeName")) ) {
		return new MGE::WebBrowser(xmlAttrib.as_string(), resX, resY, url, MGE::InteractiveTexture::OnOgreObject, context->scnMgr);
	} else {
		LOG_WARNING("Can't determined MGE::InteractiveTexture::Mode for WebBrowser");
		return nullptr;
	}
}

MGE_CONFIG_PARSER_MODULE_FOR_XMLTAG(WebBrowser) {
	return MGE::WebBrowser::create(xmlNode, context);
}


std::string MGE::WebBrowser::parseUrl(const std::string_view& url) {
	if (url.substr(0,8) == "rpath://") {
		return std::string("file://" + MGE::Engine::getPtr()->getWorkingDir() + "/" + url.substr(8));
	} else {
		return std::string(url);
	}
}

bool MGE::WebBrowser::parseUrl(
	const std::string_view& url, 
	std::string& scheme, std::string& path, std::map<std::string, std::string>& query,
	const std::set<std::string>& needScheme
) {
	LOG_DEBUG("url: " << url);
	
	size_t sep = url.find(":");
	scheme = url.substr(0, sep);
	
	bool ret = true;
	for (auto& iter : needScheme) {
		if (iter == scheme)
			ret = false;
	}
	if (ret)
		return false;
	
	std::string_view urlPath = url.substr(sep+1);
	sep = urlPath.find("?");
	if (sep != std::string_view::npos) {
		std::string_view queryStr = urlPath.substr(sep+1);
		urlPath = urlPath.substr(0, sep);
		
		do {
			sep = queryStr.find("&");
			std::string_view queryEntryStr = queryStr.substr(0, sep);
			
			size_t sep2 = queryEntryStr.find("=");
			query[ decodeUrl(queryEntryStr.substr(0, sep2)) ] = decodeUrl(queryEntryStr.substr(sep2+1));
			
			queryStr = queryStr.substr(sep+1);
		} while(sep != std::string_view::npos);
	}
	
	path = decodeUrl(urlPath);
	
	#ifdef MGE_DEBUG
	LOG_DEBUG("scheme=" << scheme << "  path=" << path << "  query:");
	for (auto& iter : query) {
		LOG_DEBUG(" * " << iter.first << " = " + iter.second);
	}
	#endif
	
	return true;
}

std::string MGE::WebBrowser::decodeUrl(const std::string_view& url) {
	std::string ret;
	ret.reserve(url.length());
	const char* c = url.data();
	const char* e = c + url.length();
	char hexchar[3];
	hexchar[2] = 0;
	
	while (c != e) {
		if (*c == '%') {
			hexchar[0] = *(++c);
			hexchar[1] = *(++c);
			ret.push_back( strtoul(hexchar, NULL, 16) );
		} else {
			ret.push_back(*c);
		}
		++c;
	}
	return ret;
}

int MGE::WebBrowser::webBrowserObjectCount = 0;



bool MGE::WebBrowser::frameStarted(const Ogre::FrameEvent& evt) {
#ifdef USE_CEF
	CefDoMessageLoopWork();
#endif
	return true;
}

bool MGE::WebBrowser::sendMouseEvent(
	const Ogre::Vector2& mouseTexturePos, OIS::MouseButtonID _buttonID, bool mouseUp
) {
#ifdef USE_CEF
	CefBrowserHost::MouseButtonType buttonID;
	switch (_buttonID) {
		case OIS::MB_Left:
			buttonID = MBT_LEFT;
			break;
		case OIS::MB_Right:
			buttonID = MBT_RIGHT;
			break;
		case OIS::MB_Middle:
			buttonID = MBT_MIDDLE;
			break;
		default:
			return true;
	}
	
	CefMouseEvent event;
	event.x = mouseTexturePos.x * renderTexture->getWidth();
	event.y = mouseTexturePos.y * renderTexture->getHeight();
	event.modifiers = 0;
	currentClient->browser->GetHost()->SetFocus(true);
	currentClient->browser->GetHost()->SendMouseClickEvent(event, buttonID, mouseUp, 1);
#endif
	return true;
}

bool MGE::WebBrowser::mousePressed(const Ogre::Vector2& mouseTexturePos, OIS::MouseButtonID buttonID, const OIS::MouseEvent& arg) {
	return sendMouseEvent(mouseTexturePos, buttonID, false);
}

bool MGE::WebBrowser::mouseMoved(const Ogre::Vector2& mousePos, const OIS::MouseEvent& arg) {
#ifdef USE_CEF
	if (arg.state.X.rel || arg.state.Y.rel) {
		std::pair<bool, Ogre::Vector2> res = textureHitTest(mousePos);
		if (res.first) {
			CefMouseEvent event;
			event.x = res.second.x * renderTexture->getWidth();
			event.y = res.second.y * renderTexture->getHeight();
			event.modifiers = 0;
			currentClient->browser->GetHost()->SetFocus(true);
			currentClient->browser->GetHost()->SendMouseMoveEvent(event, false);
		} else {
			return false;
		}
	}
#endif
	return true;
}

bool MGE::WebBrowser::mouseReleased(const Ogre::Vector2& mousePos, OIS::MouseButtonID id, const OIS::MouseEvent& arg) {
	std::pair<bool, Ogre::Vector2> res = textureHitTest(mousePos);
	if (res.first) {
		return sendMouseEvent(res.second, id, true);
	} else {
		return false;
	}
}

bool MGE::WebBrowser::keyPressed(const OIS::KeyEvent& arg) {
#ifdef USE_CEF
	CefKeyEvent key_event;
	if (arg.key == OIS::KC_RETURN) {
		key_event.character = '\r';
	} else {
		key_event.character = arg.text;
	}
	key_event.focus_on_editable_field = true;
	key_event.is_system_key           = false;
	key_event.modifiers               = 0;
	key_event.native_key_code         = key_event.character;
	key_event.unmodified_character    = key_event.character;
	key_event.windows_key_code        = key_event.character;
	key_event.type = KEYEVENT_CHAR;
	currentClient->browser->GetHost()->SendKeyEvent(key_event);
#endif
	return true;
}

bool MGE::WebBrowser::keyReleased(const OIS::KeyEvent& arg) {
	return true;
}

void MGE::WebBrowser::resize(int xSize, int ySize) {
#ifdef USE_CEF
	resizeTexture(xSize, ySize);
	
	mainClient->browser->GetHost()->WasResized();
	dialogClient->browser->GetHost()->WasResized();
#endif
}



void MGE::WebBrowser::loadURL(const std::string_view& url) {
#ifdef USE_CEF
	mainClient->browser->GetMainFrame()->LoadURL( parseUrl(url).c_str() );
#endif
}

void MGE::WebBrowser::loadString(const std::string_view& html) {
#ifdef USE_CEF
	mainClient->browser->GetMainFrame()->LoadURL(
		"data:text/html;base64," + CefURIEncode(CefBase64Encode(html.data(), html.length()), false).ToString()
	);
#endif
}

void MGE::WebBrowser::goBack() {
#ifdef USE_CEF
	mainClient->browser->GoBack();
#endif
}

void MGE::WebBrowser::goForward() {
#ifdef USE_CEF
	mainClient->browser->GoForward();
#endif
}

void MGE::WebBrowser::reload() {
#ifdef USE_CEF
	mainClient->browser->Reload();
#endif
}

void MGE::WebBrowser::stopLoad() {
#ifdef USE_CEF
	mainClient->browser->StopLoad();
#endif
}

bool MGE::WebBrowser::isLoading() {
#ifdef USE_CEF
	return mainClient->browser->IsLoading();
#else
	return false;
#endif
}

bool MGE::WebBrowser::hasDocument() {
#ifdef USE_CEF
	return mainClient->browser->HasDocument();
#else
	return false;
#endif
}
