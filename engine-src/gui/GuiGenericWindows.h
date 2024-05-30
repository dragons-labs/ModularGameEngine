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

#include "BaseClasses.h"
#include "StringTypedefs.h"

namespace pugi { class xml_node; }

#include <CEGUI/CEGUI.h>

namespace MGE {

/// @addtogroup GUI_Core
/// @{
/// @file

/// @brief Namespace for generic windows bases classes
namespace GenericWindows {
	
	class BaseWindow;
	
	/**
	 * @brief Factory class for BaseWindow derived windows
	 */
	class Factory :
		public MGE::TrivialSingleton<Factory>
	{
	public:
		/// get first level window by @a name or return NULL when not exists
		BaseWindow* get(const std::string_view& name);
		
		/// get first level window by @a name or create with factory of BaseWindow (@ref create)
		BaseWindow* get(const std::string_view& name, const std::string_view& type, const CEGUI::String& layout);
		
		/// get first level window by info from XML or create with factory of BaseWindow (@ref create)
		BaseWindow* get(const pugi::xml_node& xmlNode);
		
		/// factory of BaseWindow
		BaseWindow* create(const std::string_view& type, const CEGUI::String& layout);
		
	protected:
		friend class BaseWindow;
		
		/// map of all BaseWindow with names
		std::map<std::string, MGE::GenericWindows::BaseWindow*, std::less<>> baseWindowsMap;
	};
	
	/**
	 * @brief base window interface class
	 */
	class BaseWindow : MGE::NoCopyableNoMovable {
	public:
		/// constructor
		BaseWindow(const CEGUI::String& layout, const std::string_view& moduleName = MGE::EMPTY_STRING_VIEW, CEGUI::Window* parent = NULL);
		
		/// destructor
		virtual ~BaseWindow(void);
		
		/// unminimalize or show window
		/// on multisubwindow case identify by @a name
		virtual void show(const CEGUI::String& name = CEGUI::String::GetEmpty()) {
			window->show();
			window->activate();
		}
		
		/// minimalize, close or hide window
		virtual void hide() {
			window->hide();
		}
		
		/// switch hidding window
		virtual void switchHide() {
			if ( isHide() )
				show();
			else
				hide();
		}
		
		/// return true if window is hide (minimalized)
		virtual bool isHide() {
			return ! window->isVisible();
		}
		
		/// return pointer to cegui window object
		CEGUI::Window* getWindow() {
			return window;
		}
		
		/// increase client counter
		void addClient() {
			++numOfClients;
		}
		
		/// decrease client counter and (when is 0) remove this window
		void remClient();
		
	protected:
		/// pointer to CEGUI window
		CEGUI::Window* window;
		
		/// number of client (e.g. BaseWindowOwner objects using this window)
		int numOfClients;
	};
	
	/**
	 * @brief base class for gui elements owned BaseWindow object
	 */
	class BaseWindowOwner : MGE::NoCopyableNoMovable {
	public:
		/// constructor
		BaseWindowOwner(BaseWindow* w) {
			window = w;
			window->addClient();
		}
		
		/// destructor
		virtual ~BaseWindowOwner() {
			window->remClient();
		}
		
		/// @copydoc MGE::GenericWindows::BaseWindow::show
		virtual void show(const CEGUI::String& name = CEGUI::String::GetEmpty()) = 0;
		
		/// @copydoc MGE::GenericWindows::BaseWindow::hide
		void hide() {
			window->hide();
		}
		
		/// @copydoc MGE::GenericWindows::BaseWindow::switchHide
		void switchHide() {
			window->switchHide();
		}
		
		/// @copydoc MGE::GenericWindows::BaseWindow::isHide
		bool isHide() {
			return window->isHide();
		}
		
		/// @copydoc MGE::GenericWindows::Factory::getPtr()->getWindow
		CEGUI::Window* getWindow() {
			return window->getWindow();
		}
		
		/// return pointer to BaseWindow object
		BaseWindow* getBaseWindow() {
			return window;
		}
		
	protected:
		/// pointer to base window
		BaseWindow* window;
	};
	
	/**
	 * @brief Window with minimization option
	 */
	class MinimizableWindow : public MGE::GenericWindows::BaseWindow
	{
	public:
		/// constructor
		MinimizableWindow(const CEGUI::String& layoutFile);
		
		/// @copydoc MGE::GenericWindows::BaseWindow::show
		/// unminimalize window
		void show(const CEGUI::String& name = CEGUI::String::GetEmpty()) override;
		
		/// @copydoc MGE::GenericWindows::BaseWindow::hide
		/// minimalize window
		void hide() override;
		
		/// @copydoc MGE::GenericWindows::BaseWindow::isHide
		bool isHide() override {
			return hidden;
		}
		
	protected:
		/**
		 * @brief handle hide window
		 * 
		 * @param[in] args - OIS Event detail/description
		 */
		bool handleHide(const CEGUI::EventArgs& args);
		
	private:
		CEGUI::UVector2  normalPosition;
		CEGUI::UVector2  hidePosition;
		CEGUI::USize     defaultSize;
		bool             hidden;
	};
	
	/**
	 * @brief Window with closable option
	 */
	class ClosableWindow : public MGE::GenericWindows::BaseWindow
	{
	public:
		/// constructor
		ClosableWindow(const CEGUI::String& layoutFile);
		
	protected:
		/**
		 * @brief handle hide window
		 * 
		 * @param[in] args - OIS Event detail/description
		 */
		bool handleClose(const CEGUI::EventArgs& args);
	};
	
	/**
	 * @brief Window for tabs
	 */
	class TabsWindow : public MGE::GenericWindows::ClosableWindow
	{
	public:
		/// constructor
		TabsWindow(const CEGUI::String& layoutFile);
		
		/// switch to named tab
		bool switchToTab(const CEGUI::String& name);
		
		/// @copydoc MGE::GenericWindows::BaseWindow::show
		void show(const CEGUI::String& name = CEGUI::String::GetEmpty()) override {
			if (!name.empty())
				switchToTab(name);
			MGE::GenericWindows::BaseWindow::show(name);
		}
	
	protected:
		/**
		 * @brief handle tab switch button
		 * 
		 * @param[in] args - OIS Event detail/description
		 */
		bool handleTabSwitch(const CEGUI::EventArgs& args);
		
	private:
		CEGUI::Window* currTab;
	};
}

/// @}

}
