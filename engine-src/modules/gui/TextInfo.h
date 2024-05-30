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
#include "gui/GuiGenericWindows.h"

#include <list>

namespace MGE { class WebBrowser; }

namespace MGE {

/// @addtogroup Modules
/// @{
/// @file

/**
 * @brief Text (or HTML, etc) raport object
 */
struct TextReport {
	/// raport types
	enum ReportType {
		/// text report (support CEGUI formating tags)
		TXT,
		/// HTML report (use @ref header an @ref footer when pagging, when non empty)
		HTML,
		/// set of URLs
		URL
	};
	
	/// list of message in report
	std::list<std::string> entries;
	
	/// raport name
	std::string name;
	
	/// header string for HTML report
	std::string header;
	
	/// footer string for HTML report
	std::string footer;
	
	/// report type: TXT, HTML or URL
	ReportType type;
	
	/// number of message per page (when using autoSplit should be big)
	int  msgPerPage;
	
	/// automatic split report per pages
	bool autoSplit;
	
	/// automatic word wrap
	bool wordWrap;
	
	/// block print duplicated entries on goto previous page
	bool noDuplicatedOnPrev;
	
	/// when true display report in reverse order (from std::list::back() to std::list::front())
	bool displayFromBack;
	
	/// when true add message at begin of list
	bool addToFront;
	
	/// default value of automatic add new line at end of adding message in addMessage
	bool defaultAutoNewLine;
	
	/**
	 * @brief add message to report
	 * 
	 * @param msg         message text
	 * @param autoNewLine when true automatic add new line at end of @a msg when @a msg not ending by '\n'
	 */
	void addMessage(const std::string_view& msg, bool autoNewLine);
	
	/**
	 * @brief add message to report, use defaultAutoNewLine setting
	 * 
	 * @param msg         message text
	 */
	void addMessage(const std::string_view& msg) {
		addMessage(msg, defaultAutoNewLine);
	}
	
	/// constructor
	TextReport(const std::string_view& _name, ReportType _type = TXT);
	
	/// @copydoc MGE::SaveableToXMLInterface::restoreFromXML
	bool restoreFromXML(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context);
	
	/// @copydoc MGE::SaveableToXMLInterface::storeToXML
	bool storeToXML(pugi::xml_node& xmlNode, bool onlyRef) const;
};

/**
 * @brief Text multipage window for report, dialog history, etc
 */
class TextInfo :
	public MGE::GenericWindows::BaseWindowOwner,
	public MGE::SaveableToXML<TextInfo>,
	public MGE::Module,
	public MGE::Singleton<MGE::TextInfo>
{
public:
	/// @copydoc MGE::GenericWindows::BaseWindowOwner::show
	void show(const CEGUI::String& name = CEGUI::String::GetEmpty()) override;
	
	/**
	 * @brief return pointer to registered report object by @a name
	 * 
	 * @param name   name of report
	 * @param create when true (default) and report with @a name don't exist in TextInfo create new report, registered it and return pointer to it
	 * 
	 * @return pointer to registered report object, when @a create is false and report don't exist return NULL
	 */
	MGE::TextReport* getReport(const std::string_view& name, bool create = true);
	
	/**
	 * @brief set current report to report specified by @a name
	 * 
	 * @param name   name of report
	 * 
	 * @return true when report with @a name exist, false otherwise
	 */
	bool setCurrentReport(const std::string_view& name);
	
	/**
	 * @brief set current report to @a report
	 * 
	 * @param report pointer to report obcet to use as current report
	 */
	void setCurrentReport(MGE::TextReport* report);
	
	/**
	 * @brief inform TextInfo about change in report object, when it is current report this might result refresh displayed text
	 * 
	 * @param report pointer to report obcet to use as current report
	 * @param force  when true reinit even if not in first page
	 */
	void onReportUpdate(MGE::TextReport* report, bool force = false);
	
	/// Name of XML tag for @ref MGE::SaveableToXML::getXMLTagName.
	inline static const char* xmlStoreRestoreTagName = "TextInfo";
	
	/// @copydoc MGE::SaveableToXMLInterface::restoreFromXML
	virtual bool restoreFromXML(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context) override;
	
	/// @copydoc MGE::SaveableToXMLInterface::storeToXML
	virtual bool storeToXML(pugi::xml_node& xmlNode, bool onlyRef) const override;
	
	/// constructor - based on BaseWindow object
	TextInfo(MGE::GenericWindows::BaseWindow* _baseWin, const std::string_view& _winName = "TextInfo"sv);
	
	/**
	 * @brief create TextInfo based on XML configuration
	 * 
	 * @param[in] xmlNode           XML configuration node
	 * @param[in] context           creation context (provides access to SceneManager, etc)
	 */
	static TextInfo* create(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context);
	
	/// destructor
	virtual ~TextInfo();
	
protected:
	/// list of all available reports
	std::list<MGE::TextReport*>       reports;
	
	/// pointer to current (active, selected) report
	MGE::TextReport*                  currentReport;
	/// iterator indicating first message on current page
	std::list<std::string>::iterator  pagedTextStart;
	/// iterator indicating first message after current page
	std::list<std::string>::iterator  pagedTextEnd;
	
	/**
	 * @brief return iteartor increment by @a count or auto detected number,
	 *        optional can show crossed element
	 * 
	 * @param first        iterator to first (start) element
	 * @param count        count of element to move iterator
	 * @param btn          if not NULL disable this button when reached @a end element
	 * @param show         if true call show() on string containing all (including @a first, but not including @a end) crossed element
	 * @param noAutoSplit  if false stop before @a count step, and before reach @a end when text area is full
	 * @param end          stop on end element (default currentReport->entries.end())
	 * 
	 * @return return iterator indicating first message after printing range
	 */
	std::list<std::string>::iterator goForward (
		std::list<std::string>::iterator first,
		int count,
		CEGUI::PushButton* btn,
		bool show,
		bool noAutoSplit,
		std::list<std::string>::iterator end = getPtr()->currentReport->entries.end()
	);
	
	/**
	 * @brief return iteartor decrement by @a count or auto detected number,
	 *        optional can show crossed element
	 * 
	 * @param iter         iterator to first (start) element
	 * @param count        count of element to move iterator
	 * @param btn          if not NULL disable this button when reached @a end element
	 * @param show         if true call show() on string containing all (not including @a first, but possible including @a end) crossed element
	 * @param noAutoSplit  if false stop before @a count step, and before reach @a end when text area is full
	 * @param end          stop on end element (default currentReport->entries.begin())
	 * 
	 * @return return iterator indicating first message after printing range
	 */
	std::list<std::string>::iterator goBack (
		std::list<std::string>::iterator iter,
		int count,
		CEGUI::PushButton* btn,
		bool show,
		bool noAutoSplit,
		std::list<std::string>::iterator end = getPtr()->currentReport->entries.begin()
	);
	
	/// print next portion of messages respect currentReport->displayFromBack and etc
	void printNext();
	/// print previous portion of messages respect currentReport->displayFromBack and etc
	void printPrev();
	/// init new @ref currentReport
	void initReport();
	/// display report page
	void display(const CEGUI::String& outBuf);
	
	/// pointer to "next" button object
	CEGUI::PushButton*        nextButton;
	/// pointer to "previous" button object
	CEGUI::PushButton*        prevButton;
	/// pointer to report selection combobox
	CEGUI::Combobox*          reportSelection;
	/// pointer to text area object
	CEGUI::Window*            textBox;
	/// pointer to html area object
	CEGUI::Window*            htmlBox;
	/// size of @ref htmlBox for which it was prepared @ref htmlBrowser texture
	CEGUI::Sizef              htmlBoxSize;
	/// pointer to HTML browser object
	MGE::WebBrowser* htmlBrowser;
	
	/// handle button click and show next page
	bool handleNext(const CEGUI::EventArgs& args);
	/// handle button click and show previous page
	bool handlePrev(const CEGUI::EventArgs& args);
	/// handle resize window (when using auto page split recalculate page size)
	bool handleResize(const CEGUI::EventArgs& args);
	/// handle select report from combobox
	bool handleRaportName(const CEGUI::EventArgs& args);
	
	/// (sub)window name for show() function
	std::string winName;
};

/// @}

}
