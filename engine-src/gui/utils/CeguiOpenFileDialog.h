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
#include "config.h"

#include "StringTypedefs.h"

#include <CEGUI/CEGUI.h>

#include <list>
#include <filesystem>

namespace MGE {

/// @addtogroup GUI_Utils
/// @{
/// @file

/**
 * @brief Open file dialog
 */
class OpenFileDialog
{
public:
	/**
	 * @brief constructor
	 * 
	 * @param win          CEGUI window to operate on it (must have proper child "FileList", "FullPath" and "FileNameEdit")
	 * @param filesColour  color of standard file entry in "FileList"
	 * @param dirsColour   color of directory entry in "FileList"
	 */
	OpenFileDialog(CEGUI::Window* win, const CEGUI::Colour& filesColour = 0xff000000, const CEGUI::Colour& dirsColour = 0xff0000ff);
	
	/**
	 * @brief show file-load window with @a dir directory
	 * 
	 * @param[in] baseDirPath    base directory path (can't be empty, use "." for current directory or "/" for root directory)
	 * @param[in] baseDirName    base directory display name
	 * @param[in] proposedPath   proposed filename to save (can contain directories path - using "/" as separator)
	 *                           can be used also in load mode for default directory (should be relative to dirPath path end with "/")
	 * @param[in] saveMode       if true run in "save" window mode
	 */
	void show(
		const std::string& dirPath = ".",
		const std::string& dirName = MGE::EMPTY_STRING,
		const std::string& proposedPath = MGE::EMPTY_STRING,
		bool  saveMode = false
	);
	
	/**
	 * @brief reload on current directory
	 */
	void reload();
	
	/**
	 * @brief hide file-load window
	 */
	void hide() {
		mainWin->hide();
	}
	
	/**
	 * @brief return selected (load mode) file path or EMPTY string when no selected file
	 */
	std::string getSelectedFile();
	
	/**
	 * @brief create and return full save path (not existed directory will be created)
	 */
	std::string createSavePath();
	
	
	/**
	 * @brief Return curent path (path of display directory) adding @a prefix and @a surfix.
	 * 
	 * @param[in] prefix  String to add at begin of current path (e.g. root dir path or protocol string)
	 *                     instead of "base directory" (first element of @ref filesListCurrPath).
	 * @param[in] surfix  String to add at end of current path (e.g. filename).
	 */
	std::string getCurentPath(
		const std::string_view& prefix = MGE::EMPTY_STRING_VIEW,
		const std::string_view& surfix = MGE::EMPTY_STRING_VIEW
	);
	
protected:
	/**
	 * @brief handle file selection change and update file name edit box
	 * 
	 * @param[in] args  OIS Event detail/description
	 */
	bool filesListSelectionChanged(const CEGUI::EventArgs& args);
	
	/**
	 * @brief handle file double click change and enter to sub-directory
	 * 
	 * @param[in] args  OIS Event detail/description
	 */
	bool filesListDoubleClick(const CEGUI::EventArgs& args);
	
	/**
	 * @brief read @a dir directory to file list
	 * 
	 * @param[in] pDir  directory to read
	 */
	void readDirToFileList(const std::filesystem::path& pDir);
	
private:
	std::list<std::string>      filesListCurrPath;
	std::string                 baseDirPrettyName;
	
	CEGUI::Window*              mainWin;
	CEGUI::MultiColumnList*     filesList;
	CEGUI::Window*              fileFullPath;
	CEGUI::Editbox*             fileNameEditBox;
	
	const CEGUI::Colour         stdColour;
	const CEGUI::Colour         dirColour;
};

/// @}

}
