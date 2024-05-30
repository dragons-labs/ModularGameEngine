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

#include "gui/utils/CeguiOpenFileDialog.h"

#include "LogSystem.h"
#include "gui/InputAggregator4CEGUI.h"

#include <chrono>

void MGE::OpenFileDialog::show(const std::string& baseDirPath, const std::string& baseDirName, const std::string& proposedPath, bool saveMode) {
	LOG_INFO("OpenFileDialog::show baseDirPath=" << baseDirPath << " baseDirName=" << baseDirName << " proposedPath" << proposedPath << " saveMode" << saveMode);
	
	filesListCurrPath.clear();
	filesListCurrPath.push_back(baseDirPath); // put "base directory" as first element of filesListCurrPath
	
	baseDirPrettyName = baseDirName; // store pretty name for "base directory" (eg. pseudo protocol)
	
	auto dPath = std::filesystem::path(baseDirPath); //< current directory path
	if(!proposedPath.empty()) {
		std::filesystem::path pPath = std::filesystem::path(proposedPath).relative_path(); //< proposed file path
		std::filesystem::path npPath; //< new proposed file path → without existing as directory tree path prefix
		for (const auto& it : pPath) {                                 // 1. get front elements of proposedPath
			if (npPath.empty()) {
				auto tmpPath = dPath / it;
				if (std::filesystem::is_directory(tmpPath)) {          // 2. while (with current directory path) they make path to existing directory
					filesListCurrPath.push_back(it.generic_string());  //   → add it to filesListCurrPath (as separate elements)
					dPath = tmpPath;                                   //   → add it to dPath
					continue;
				}                                                      // 3. if they already aren't path to existing directory
			}
			npPath /= it;                                              // 4. add they to npPath (adding the first one will stop checking for existence)
		}
		fileNameEditBox->setText( npPath.generic_string() );           // 5. set npPath (not existing part of proposedPath) in edit box
	}
	readDirToFileList( dPath );                                        // 6. use dPath (baseDirPath + existing part of proposedPath)
	
	if (saveMode) {
		mainWin->getChild( "OpenFileDialog : Save" )->setProperty("Disabled", "False");
		fileNameEditBox->show();
	} else {
		mainWin->getChild( "OpenFileDialog : Save" )->setProperty("Disabled", "True");
		fileNameEditBox->hide();
	}
	
	mainWin->show();
}

std::string MGE::OpenFileDialog::getCurentPath(const std::string_view& prefix, const std::string_view& surfix) {
	std::string path;
	
	// when non empty prefix - replace in returned path first element of filesListCurrPath ("base directory") by prefix
	if (!prefix.empty()) {
		path.append(prefix);
	} else {
		path.append(filesListCurrPath.front() + "/");
	}
	
	auto iter = filesListCurrPath.begin();
	while (++iter != filesListCurrPath.end()) { // don't use `for(auto& it : set)` because we start from 2nd element of filesListCurrPath (important when prefix != "")
		path.append(*iter + "/");
	}
	
	if (!surfix.empty()) {
		path.append(surfix);
	}
	
	LOG_VERBOSE("getCurentPath return: " << path);
	return path;
}

std::string MGE::OpenFileDialog::getSelectedFile() {
	CEGUI::ListboxItem* item = filesList->getFirstSelectedItem();
	if (item) {
		return getCurentPath(MGE::EMPTY_STRING_VIEW, item->getText().getString());
	} else {
		return MGE::EMPTY_STRING;
	}
}

std::string MGE::OpenFileDialog::createSavePath() {
	std::string fullPath = getCurentPath() + fileNameEditBox->getText().getString();
	std::filesystem::create_directories( std::filesystem::path(fullPath).parent_path() );
	return fullPath;
}

void MGE::OpenFileDialog::reload() {
	readDirToFileList( std::filesystem::path( getCurentPath() ) );
	fileNameEditBox->setText("");
}

void MGE::OpenFileDialog::readDirToFileList(const std::filesystem::path& pDir) {
	CEGUI::ListboxTextItem* item;
	int rowNum;
	char buf[32];
	
	fileFullPath->setText( getCurentPath(baseDirPrettyName) );
	
	filesList->resetList();
	filesList->setSortColumn(0);
	const CEGUI::String& brushImage = filesList->getProperty("DefaultItemSelectionBrushImage");
	
	if (filesListCurrPath.size() > 1) {
		rowNum = filesList->addRow();
		item = new CEGUI::ListboxTextItem("..", rowNum);
		item->setTextColours(dirColour);
		item->setSelectionBrushImage(brushImage);
		item->setAutoDeleted(true);
		filesList->setItem( item, 0, rowNum);
		
		item = new CEGUI::ListboxTextItem("LEVEL UP", rowNum);
		item->setTextColours(dirColour);
		item->setSelectionBrushImage(brushImage);
		item->setAutoDeleted(true);
		filesList->setItem( item, 1, rowNum);
	}
	

	for (auto const& dirEntry : std::filesystem::directory_iterator{pDir}) {
		rowNum = filesList->addRow();
		
		// filename
		item = new CEGUI::ListboxTextItem(dirEntry.path().filename().generic_string(), rowNum);
		if (std::filesystem::is_directory(dirEntry.path()))
			item->setTextColours(dirColour);
		else
			item->setTextColours(stdColour);
		item->setSelectionBrushImage(brushImage);
		item->setAutoDeleted(true);
		filesList->setItem( item, 0, rowNum);
		
		// time
		auto time = std::chrono::system_clock::to_time_t(
			std::chrono::file_clock::to_sys(
				std::filesystem::last_write_time(dirEntry.path())
			)
		);
		std::strftime( buf, 32, "%Y-%m-%d %H:%M:%S", std::localtime(&time) );
		item = new CEGUI::ListboxTextItem(buf, rowNum);
		item->setTextColours(stdColour);
		item->setSelectionBrushImage(brushImage);
		item->setAutoDeleted(true);
		filesList->setItem( item, 1, rowNum);
	}
	
	filesList->setSortDirection(CEGUI::ListHeaderSegment::SortDirection::Descending);
	filesList->setSortColumn(1);
}

bool MGE::OpenFileDialog::filesListDoubleClick(const CEGUI::EventArgs& args) {
	auto mbargs = static_cast<const CEGUI::MouseButtonEventArgs&>(args);
	
	LOG_DEBUG("OpenFileDialog::filesListDoubleClick " << mbargs.d_generatedClickEventOrder);
	
	if (mbargs.d_generatedClickEventOrder == 2) {
		CEGUI::ListboxItem* item = filesList->getFirstSelectedItem();
		if (item) {
			auto sDir = item->getText().getString();
			std::filesystem::path pDir( getCurentPath(MGE::EMPTY_STRING_VIEW, sDir) );
			
			if (std::filesystem::is_directory(pDir)) {
				if (sDir == "..")
					filesListCurrPath.pop_back();
				else
					filesListCurrPath.push_back(sDir);
				readDirToFileList(pDir);
			}
		}
	}
	return true;
}

bool MGE::OpenFileDialog::filesListSelectionChanged(const CEGUI::EventArgs& args) {
	CEGUI::ListboxItem* item = filesList->getFirstSelectedItem();
	if (item) {
		auto sDir = item->getText().getString();
		std::filesystem::path pDir( getCurentPath(MGE::EMPTY_STRING_VIEW, sDir) );
		if (! std::filesystem::is_directory(pDir)) {
			fileNameEditBox->setText( item->getText() );
		}
	}
	return true;
}

MGE::OpenFileDialog::OpenFileDialog(
	CEGUI::Window* win, const CEGUI::Colour& filesColour, const CEGUI::Colour& dirsColour
)
	: mainWin(win), stdColour(filesColour), dirColour(dirsColour)
{
	filesList       = static_cast<CEGUI::MultiColumnList*>(mainWin->getChild( "FileList" ));
	fileFullPath    = mainWin->getChild( "FullPath" );
	fileNameEditBox = static_cast<CEGUI::Editbox*>(mainWin->getChild( "FileNameEdit" ));
	
	filesList->addColumn("Name", 0, CEGUI::UDim(0.65f, 0));
	filesList->addColumn("Time", 1, CEGUI::UDim(0.35f, -16));
	filesList->setSelectionMode(CEGUI::MultiColumnList::SelectionMode::RowSingle);
	// filesList->getListHeader()->setHeight(CEGUI::UDim(0.0f, 0));
	filesList->subscribeEvent(
		CEGUI::MultiColumnList::EventSelectionChanged, CEGUI::Event::Subscriber(&MGE::OpenFileDialog::filesListSelectionChanged, this)
	);
	filesList->subscribeEvent(
		CEGUI::Window::EventClick,
		CEGUI::Event::Subscriber(&MGE::OpenFileDialog::filesListDoubleClick, this)
	);
}
