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

#include "ModuleBase.h"

#include "gui/GuiSystem.h"
#include "rendering/audio-video/AudioSystem.h"

namespace MGE { struct TextReport; }

namespace MGE {

/// @addtogroup Modules
/// @{
/// @file

/**
 * @brief Text (with audio support) dialog menu
 */
class TextDialog MGE_CLASS_FINAL :
	public MGE::SaveableToXML<TextDialog>,
	public MGE::Module,
	public MGE::Singleton<TextDialog>
{
public:
	/**
	 * @brief initialize dialog / run next dialog step
	 * 
	 * @param[in] initScript      dialog script to run
	 * @param[in] initStep        numeric argument for dialog script (dialog step)
	 * @param[in] autopause       when true pause game for dialog
	 */
	void runDialog(const std::string_view& initScript, int initStep, bool autopause);
	
	/**
	 * @brief initialize dialog / run next dialog step
	 * 
	 * @param[in] initScript      dialog script to run
	 * @param[in] initStep        numeric argument for dialog script (dialog step)
	 */
	void runDialog(const std::string_view& initScript, int initStep);
	
	/**
	 * @brief show dialog text
	 * 
	 * @param[in] text            dialog text to show
	 * @param[in] audio           dialog audio file name to play
	 * @param[in] timeout         to clear dialog text or run callbackScript (if set)
	 *                            when timeout == 0 @a callbackScript and @a step will be ignored
	 * @param[in] callbackScript  script name to run after timeout, see @ref MGE::TextDialog::nextScript
	 *                            if empty clear text Box after timeout
	 *                            if non-empty enter to dialog mode and pause game
	 * @param[in] step            dialog step to pass to callbackScript via doDialogStep call
	 */
	void showText(const std::string_view& text, const Ogre::String& audio = Ogre::BLANKSTRING, int timeout = 0, const std::string_view& callbackScript = MGE::EMPTY_STRING_VIEW, int step = 0);
	
	
	/**
	 * @brief load (if need) and set image in image window, if no image window add it
	 * 
	 * @param[in] name        file name with image
	 * @param[in] group       resources group name for image
	 */
	void setImage(const CEGUI::String& name, const CEGUI::String& group);
	
	/**
	 * @brief unset and (optional) unload image from image window
	 * 
	 * @param[in] hide        when true hide image window
	 * @param[in] unload      when true unload image
	 */
	void unsetImage(bool hide = true, bool unload = false);
	
	
	/**
	 * @brief add answer options to dialog, using color from xml description of answer window
	 * 
	 * @param[in] text        answer option text
	 * @param[in] id          answer option id (passed to callbackScript)
	 */
	void addAnswer(const CEGUI::String& text, int id);
	
	/**
	 * @brief show dialog answer window with previously added answers options and wait for answer
	 * 
	 * @param[in] callbackScript  script name to run after selected answer, see @ref MGE::TextDialog::nextScript
	 */
	void showAnswers(const std::string_view& callbackScript);
	
	
	/**
	 * @brief return true if dialog is working
	 */
	inline bool onDialog() const {
		return currState != OFF;
	}
	
	/// Name of XML tag for @ref MGE::SaveableToXML::getXMLTagName.
	inline static const char* xmlStoreRestoreTagName = "TextDialog";
	
	/// @copydoc MGE::SaveableToXMLInterface::restoreFromXML
	virtual bool restoreFromXML(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context) override;
	
	/// @copydoc MGE::SaveableToXMLInterface::storeToXML
	virtual bool storeToXML(pugi::xml_node& xmlNode, bool onlyRef) const override;
	
	/**
	 * @brief constructor - using file window setting
	 * 
	 * @param win              pointer to message window
	 * @param log              pointer to TextReport object, where it will be stored message history
	 * @param autopause       set default value of auto pause for runDialog
	 * @param autohide         when true auto hide text message bar
	 */
	TextDialog(CEGUI::Window* win, MGE::TextReport* log = NULL, bool autohide = true, bool autopause = true);
	
	/**
	 * @brief constructor - using file window setting
	 * 
	 * @param dialogWinLayout  layout filename for message window
	 * @param log              pointer to TextReport object, where it will be stored message history
	 * @param autohide         when true auto hide text message bar
	 * @param autopause       set default value of auto pause for runDialog
	 * @param parent           pointer to parrent window (when NULL using default parrent window)
	 */
	TextDialog(const CEGUI::String& dialogWinLayout, MGE::TextReport* log = NULL, bool autohide = true, bool autopause = true, CEGUI::Window* parent = NULL);
	
	/**
	 * @brief constructor - using file window setting
	 * 
	 * @param log              pointer to TextReport object, where it will be stored message history
	 * @param autohide         when true auto hide text message bar
	 * @param autopause       set default value of auto pause for runDialog
	 * @param parent           pointer to parrent window (when NULL using default parrent window)
	 */
	TextDialog(MGE::TextReport* log = NULL, bool autohide = true, bool autopause = true, CEGUI::Window* parent = NULL);
	
	/**
	 * @brief create TextDialog based on XML configuration
	 * 
	 * @param[in] xmlNode           XML configuration node
	 */
	static TextDialog* create(const pugi::xml_node& xmlNode);
	
	/// destructor
	virtual ~TextDialog();
	
protected:
	/// dialog states
	enum DialogStates {
		/// wait for status update from script (defined by currScript and currStep)
		RUN_SCRIPT,
		/// wait for finish display dialog text and run nextScript with nextStep
		SHOW_TEXT,
		/// wait for user input and run nextScript with id depending on user choice
		WAIT_FOR_ANSWER,
		/// dialog is ended
		OFF
	};

	/// current dialog state
	DialogStates                  currState;
	/// name of script that created the current state of dialog window
	std::string                   currScript;
	/// numeric argument to pass @ref currScript for create the current state of dialog window
	int                           currStep;
	/// name of current showing image
	CEGUI::String                 currImage;
	/// group of current showing image
	CEGUI::String                 currImageGroup;
	
	/// name of sctipt to call after timeout, click or answer (empty for end dialog)
	std::string                   nextScript;
	/// numeric argument to pass nextScript (dialog step), not used when wait for answer
	int                           nextStep;
	/// true when dialog call pause()
	bool                          startPause;
	
	/// default value for autopause in @ref runDialog
	bool                          defaultAutoPause;
	/// if true hide dialog bar window when no running dialog
	bool                          autoHideDialogWin;
	
	/// pointer to TextReport for storing dialog history (when NULL disable this feature)
	MGE::TextReport*              logReport;
	
	/// pointer to parent window
	CEGUI::Window*                parentWin;
	/// pointer to dialog main window
	CEGUI::Window*                dialogWin;
	/// pointer to dialog text window
	CEGUI::MultiLineEditbox*      textBox;
	/// pointer to dialog image window
	CEGUI::Window*                imageBox;
	/// pointer to dialog answer window
	CEGUI::Window*                answerBox;
	/// pointer to combobox in dialog answer window
	CEGUI::ListWidget*            answerList;
	/// pointer to confirm button in dialog answer window
	CEGUI::Window*                answerButton;
	/// base position of dialog text window (used to calculate position after add/hide image)
	CEGUI::UDim                   baseXPosition;
	/// base width of dialog text window (sed to calculate width after add/hide image)
	CEGUI::UDim                   baseWidth;
	
	#ifdef USE_OGGSOUND
	/// pointer to dialog audio object
	OgreOggSound::OgreOggISound*  dialogSound;
	#endif
	
	/// callback function for click on dialog text window for skip dialog entry
	bool handleClick(const CEGUI::EventArgs& args);
	
	/// callback function for select and confirm dialog answer
	bool handleAnswer(const CEGUI::EventArgs& args);
	
	/// callback function for dialog timer
	bool handleTimer();
};

/// @}

}
