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

#include "physics/Raycast_forward.h"

namespace Ogre  { class Vector2; }
namespace CEGUI { class Window; }

namespace MGE {

/// @addtogroup Input
/// @{
/// @file

/**
 * @brief Base (interface) class for context menu.
 */
class SelectionContextMenu {
public:
	/**
	 * @brief show context menu at position mousePos, relative to window tgrWin (or defaultParent if tgrWin is NULL)
	 * 
	 * @param[in] mousePos     screen point to show context menu (click point)
	 * @param     tgrWin       pointer to window for which mousePos is relative and has be parrent window of menu
	 * @param     clickSearch  smart pointer to search results object related to the click which opened the menu
	 *                          it contains information about menu target objects
	 */
	virtual void showContextMenu(const Ogre::Vector2& mousePos, CEGUI::Window* tgrWin, MGE::RayCast::ResultsPtr clickSearch) = 0;
	
	/**
	 * @brief hide context menu
	 */
	virtual void hideContextMenu() = 0;
	
	/// destructor
	virtual ~SelectionContextMenu() {}
};

/// @}

} 
