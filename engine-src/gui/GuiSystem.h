/*
Copyright (c) 2013-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>

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
*/

#pragma   once

#include "Engine.h"
#include "BaseClasses.h"
#include "StringTypedefs.h"
#include "ModuleBase.h"

#include <CEGUI/CEGUI.h>
#include <OgreFrameListener.h>

namespace CEGUI { class OgreRenderer; }
namespace MGE { class GUIOnTexture; }
namespace MGE { class InputAggregator4CEGUI; }

namespace MGE {

/// @addtogroup GUI_Core
/// @{
/// @file

/**
 * @brief initializing CEGUI system
 */
class GUISystem :
	public Ogre::FrameListener,
	public MGE::Module,
	public MGE::Singleton<GUISystem>
{
public:
	/// @name auxiliary utils %GUI functions
	/// @{
		/**
		 * @brief find GUI window at required position
		 * 
		 * @param[in] position    screen position where we are looking for SubView window
		 */
		static CEGUI::Window* findGUIWindow(const Ogre::Vector2& position, CEGUI::Window* parent);
		
		/**
		 * @brief Create window and attach it to @a parent or (if @a parent is null) to main window.
		 * 
		 * @param[in] layout      Layout file for created window.
		 * @param[in] moduleName  Name of module ordering the creation the window. Only for print in log.
		 * @param[in] parent      Pointer to parent.
		 */
		CEGUI::Window* createGUIWindow(const CEGUI::String& layout, const std::string_view& moduleName = MGE::EMPTY_STRING_VIEW, CEGUI::Window* parent = NULL);
		
		/**
		 * @brief set visible of mouse cursor
		 */
		void setMouseVisible(bool vis);
	/// @}
	
	/// @name retrieve %GUI system elements
	/// @{
		/**
		 * @brief return pointer to gui renderer (CEGUI::OgreRenderer)
		 */
		inline CEGUI::OgreRenderer* getRenderer() const {
			return gRenderer;
		}
		
		/**
		 * @brief return pointer to GUI main window
		 */
		inline CEGUI::Window* getMainWindow() const {
			return gMainWindow;
		}
	/// @}
	
	/// @name g11n support
	/// @{
		/**
		 * @brief set translated text to @a win window, when can't get from UserString "txt:$LANGID" on @a win, then use text provided as @a altStr
		 */
		void setTranslatedText(CEGUI::Window* win, const CEGUI::String& altStr, const std::string_view& prefix = "txt:"sv);
		
		/**
		 * @brief set translated text to @a win window, when can't get from UserString "txt:$LANGID" on @a win, then use window name instead
		 */
		FORCE_INLINE void setTranslatedText(CEGUI::Window* win) {
			return setTranslatedText( win, win->getName() );
		}
	/// @}
	
	/// @name additional gui context support
	/// @{
		/**
		 * @brief register additional gui context
		 */
		void registerContext(MGE::GUIOnTexture* gc);
		
		/**
		 * @brief unregister additional gui context
		 */
		void unregisterContext(MGE::GUIOnTexture* gc);
	/// @}
	
	/**
	 * @brief constructor - initialize GUI system
	 * 
	 * @param[in] xmlNode           XML configuration node
	 */
	GUISystem(const pugi::xml_node& xmlNode);
	
protected:
	/// destructor
	~GUISystem(void);
	
	/**
	 * @brief ogre frame listener callback
	 */
	bool frameRenderingQueued(const Ogre::FrameEvent& evt) override;
	
	/// pointer to gui renderer (CEGUI::OgreRenderer)
	CEGUI::OgreRenderer* gRenderer;
	
	/// pointer to GUI context
	CEGUI::GUIContext* gMainContext;
	
	/// pointer to GUI main window
	CEGUI::Window* gMainWindow;
	
	/// set of addional gui contexts
	std::set<MGE::GUIOnTexture*> gExtraContexts;
	
	#ifndef __DOCUMENTATION_GENERATOR__
	class MyCeguiLogger : public CEGUI::Logger {
	public:
		MyCeguiLogger();
		void logEvent(const CEGUI::String& message, CEGUI::LoggingLevel level = CEGUI::LoggingLevel::Standard) override;
	private:
		void setLogFilename(const CEGUI::String& filename, bool append = false) override {}
	};
	#endif
	
	/// pointer to class inheriting from CEGUI::Logger, which uses MGE::Utils::LogSystem for writing logs
	MyCeguiLogger* ceguiLogger;
	
	/// pointer to class implemented clipboard support
	CEGUI::NativeClipboardProvider* clipboardProvider;
};

/// @}

}
