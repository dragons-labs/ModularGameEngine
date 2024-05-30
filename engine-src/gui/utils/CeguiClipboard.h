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
*/

#pragma   once
#include "config.h"

#include <CEGUI/Clipboard.h>
#include <OgreWindow.h>
#include <string_view>

namespace MGE {

/// @addtogroup GUI_Utils
/// @{
/// @file

/**
 * @brief class providing (simple) OS native clipboard integration for CEGUI
 *        (implementation depends of used OS)
 */
class CeguiNativeClipboard : public CEGUI::NativeClipboardProvider {
	/// put @a str to clipboard
	void _sendToClipboard(const std::string_view str);
	
	/// get clipboard text to @ref content
	const std::string_view& _retrieveFromClipboard();
public:
	/// cegui sent to clipboard member function for NativeClipboardProvider
	void sendToClipboard(const CEGUI::String& mimeType, void* buffer, size_t size) override;
	
	/// cegui get from clipboard member function for NativeClipboardProvider
	void retrieveFromClipboard(CEGUI::String& mimeType, void*& buffer, size_t& size) override;
	
	/// constructor
	CeguiNativeClipboard(Ogre::Window* renderWindow);
	
	/// destructor
	~CeguiNativeClipboard();
	
	/// return true when current build OS is supported
	static bool supported();
	
private:
	struct PrivData;
	PrivData* privData;
};

/// @}

}
