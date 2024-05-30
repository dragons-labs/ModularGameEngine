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
#include "ModuleBase.h"

#include <string_view>

namespace CEGUI { class  Window; }

namespace MGE {

/// @addtogroup GUI_Modules
/// @{
/// @file

/**
 * @brief Support for on screen text info (OSD) with CEGUI
 */
class OnScreenInfo :
	public MGE::Module,
	public MGE::Singleton<OnScreenInfo>
{
public:
	/**
	 * @brief show "on screen info"
	 * 
	 * @param[in] txt         text to show
	 * @param[in] code        key to protection hidding this text by other call
	 *                        0 == no protection  -1 == show previous text (@a txt is ignored) -2 == force show this text
	 * @param[in] width       when !=0 set witdh to this value
	 */
	bool showOnScreenText(const std::string_view& txt, int code = 0, int width = 0);
	
	/**
	 * @brief hide "on screen info"
	 * 
	 * @param[in] code        key to protection hidding on screen text
	 *                        0 == no protection  -2 == force hide window
	 */
	bool hideOnScreenText(int code = 0);
	
	/**
	 * @brief return true when "on screen info" is visible
	 */
	bool isOnScreenText();
	
	/// destructor
	~OnScreenInfo();
	
	/// constructor
	OnScreenInfo();
	
protected:
	/// pointer to "on screen info" window
	CEGUI::Window* onScreenInfo;
	
	/// secret code for current "on screen info"
	int onScreenInfoCode;
};

/// @}

}
