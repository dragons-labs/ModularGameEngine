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

#include <CEGUI/CEGUI.h>

namespace MGE {

/// @addtogroup GUI_Utils
/// @{
/// @file

	/**
	 * @brief load (if need) and set image for StaticImage, ListboxImageItem, etc
	 *        image is stretching whith respecting image aspect ratio
	 * 
	 * @param imageWin    pointer to CEGUI window to set image
	 * @param imageName   name image to load and set
	 * @param imageGroup  resource group for loading image
	 * 
	 * @note for ListboxImageItem before call this function we need add item to list and set item height
	 *       otherwise getPixelSize() in function don't work correctly
	 */
	void setStretchedImage(CEGUI::Window* imageWin, const CEGUI::String& imageName, const CEGUI::String& imageGroup);
	
	/**
	 * @brief return image size respectiong aspect ratio
	 * 
	 * @param reqSize   requested size (one of dimmension can be changed)
	 * @param orgSize   orginal size
	 */
	CEGUI::Sizef getRespectRatioSize(CEGUI::Sizef reqSize, const CEGUI::Sizef& orgSize);

/// @}
}
