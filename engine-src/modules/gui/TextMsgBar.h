/*
Copyright (c) 2016-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>

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

#include "ModuleBase.h"

#include "gui/GuiSystem.h"

#include <list>

namespace MGE { struct TextReport; }

namespace MGE {

/// @addtogroup Modules
/// @{
/// @file

/**
 * @brief Text (with audio support) info bar
 */
class TextMsgBar :
	public MGE::Module,
	public MGE::SaveableToXML<TextMsgBar>,
	public MGE::Singleton<TextMsgBar>
{
public:
	/**
	 * @brief add text info to message bar
	 * 
	 * @param[in] text       text message to show
	 * @param[in] count      the number of repetitions text message
	 * @param[in] priority   message priority
	 * @param[in] colorARGB  color for answer text
	 * @param[in] audio      audio file name to play
	 */
	void addMessage(const std::string_view& text, int count = 3, int priority = 10, unsigned int colorARGB = 0x00000000, const std::string_view& audio = MGE::EMPTY_STRING_VIEW);
	
	/// Name of XML tag for @ref MGE::SaveableToXML::getXMLTagName.
	inline static const char* xmlStoreRestoreTagName = "TextMsgBar";
	
	/// @copydoc MGE::SaveableToXMLInterface::restoreFromXML
	virtual bool restoreFromXML(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context) override;
	
	/// @copydoc MGE::SaveableToXMLInterface::storeToXML
	virtual bool storeToXML(pugi::xml_node& xmlNode, bool onlyRef) const override;
	
	/**
	 * @brief constructor - using indicated window
	 * 
	 * @param win              pointer to message window
	 * @param log              pointer to TextReport object, where it will be stored message history
	 * @param autohide         when true auto hide text message bar
	 * @param refresh          refresh (text move) period in ms    
	 * @param txtExtraBufSize  extra size (more than width of the message bar) of output text buffer (in char)
	 */
	TextMsgBar(CEGUI::Window* win, MGE::TextReport* log = NULL, bool autohide = true, int refresh = 200, int txtExtraBufSize = 0);
	
	/**
	 * @brief constructor - using indicated layout file
	 * 
	 * @param msgWinLayout     layout filename for message window
	 * @param log              pointer to TextReport object, where it will be stored message history
	 * @param autohide         when true auto hide text message bar
	 * @param refresh          refresh (text move) period in ms    
	 * @param txtExtraBufSize  extra size (more than width of the message bar) of output text buffer (in char)
	 * @param parent           pointer to parrent window (when NULL using default parrent window)
	 */
	TextMsgBar(const CEGUI::String& msgWinLayout, MGE::TextReport* log = NULL, bool autohide = true, int refresh = 200, int txtExtraBufSize = 0, CEGUI::Window* parent = NULL) :
		TextMsgBar(MGE::GUISystem::getPtr()->createGUIWindow(msgWinLayout, "TextMsgBar", parent), log, autohide, refresh, txtExtraBufSize) { }
	
	/**
	 * @brief constructor - using default layout file
	 * 
	 * @param log              pointer to TextReport object, where it will be stored message history
	 * @param autohide         when true auto hide text message bar
	 * @param refresh          refresh (text move) period in ms    
	 * @param txtExtraBufSize  extra size (more than width of the message bar) of output text buffer (in char)
	 * @param parent           pointer to parrent window (when NULL using default parrent window)
	 */
	TextMsgBar(MGE::TextReport* log = NULL, bool autohide = true, int refresh = 200, int txtExtraBufSize = 0, CEGUI::Window* parent = NULL) :
		TextMsgBar("TextMsgBar.layout", log, autohide, refresh, txtExtraBufSize, parent) { }
	
	/**
	 * @brief create TextMsgBar based on XML configuration
	 * 
	 * @param[in] xmlNode           XML configuration node
	 */
	static TextMsgBar* create(const pugi::xml_node& xmlNode);
	
	/// destructor
	virtual ~TextMsgBar();
	
protected:
	/// pointer to message bar window
	CEGUI::Window*             msgWin;
	/// try refill txtOutBuf (using next message from msgQueue) when number of printable char in txtOutBuf is less than txtMinBufSize
	int                        txtMinBufSize;
	/// empty string using to start showing messages from right side of message bar window
	CEGUI::String              txtEmptyBuf;
	/// refresh (one character shift) period
	int                        refreshPeriod;
	/// pointer to TextReport for storing messages history (when NULL disable this feature)
	MGE::TextReport*  logReport;
	
	/// struct used to store single message in queue
	struct Message {
		/// message text
		std::string  txt;
		/// decremented number of repeats message
		int          count;
		/// message priority (used to determinate queue position for new message)
		int          priority;
		/// if true message is not new / was displayed (used to determinate queue position for new message)
		bool         wasShown;
		
		/// constructor
		Message(const std::string_view& t, int c, int p);
		
		/// constructor from xml
		Message(const pugi::xml_node& xmlNode);
		
		/// @copydoc MGE::SaveableToXMLInterface::storeToXML
		bool storeToXML(pugi::xml_node& xmlNode, bool onlyRef) const;
	};
	
	/// if true hide message bar window when no showing message
	bool                autoHideMsgWin;
	
	/// when true message bar window has timer (is working)
	bool                hasTimer;
	
	/// queue of messages to show
	std::list<Message*> msgQueue;
	
	/// current out (display) buffer
	CEGUI::String       txtOutBuf;
	
	/// number of printable char in txtOutBuf
	int                 txtOutBufLen;
	
	/// handle click on message bar (when logReport not NULL show report info window with logReport)
	bool handleClick(const CEGUI::EventArgs& args);
	
	/// timer callback function for refresh (shift one char) message bar
	bool refresh();
};

/// @}

}
