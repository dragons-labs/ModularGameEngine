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

#include "gui/utils/CeguiStretchedImage.h"

#include "LogSystem.h"

void MGE::setStretchedImage(CEGUI::Window* imageWin, const CEGUI::String& imageName, const CEGUI::String& imageGroup) {
	if (!imageName.empty()) {
		if (!CEGUI::ImageManager::getSingleton().isDefined(imageName)) {
			CEGUI::ImageManager::getSingleton().addBitmapImageFromFile(
				imageName,
				imageName,
				imageGroup
			);
		}
		
		CEGUI::Sizef imgSize = CEGUI::ImageManager::getSingleton().get( imageName ).getRenderedSize();
		CEGUI::Sizef winSize = imageWin->getPixelSize();
		CEGUI::URect winBox  = CEGUI::PropertyHelper<CEGUI::URect>::fromString( imageWin->getPropertyDefault("ImageArea") ); // CEGUI::Rect<CEGUI::UDim>
		
		float winWidth  = (winBox.right().d_scale * winSize.d_width + winBox.right().d_offset)
			            - (winBox.left().d_scale  * winSize.d_width + winBox.left().d_offset );
		float winHeight = (winBox.bottom().d_scale * winSize.d_height + winBox.bottom().d_offset)
			            - (winBox.top().d_scale    * winSize.d_height + winBox.top().d_offset   );
		
		float imgRatio = imgSize.d_width / imgSize.d_height;
		float winRatio = winWidth / winHeight;
		
		if (imgRatio < winRatio) {
			// reduce the width ...
			float val = (winWidth - imgRatio * winHeight) / 2.0;
			
			winBox.d_min.d_x.d_offset += val;
			winBox.d_max.d_x.d_offset -= val;
		} else {
			// reduce the height ...
			float val = (winHeight - winWidth / imgRatio) / 2.0;
			
			winBox.d_min.d_y.d_offset += val;
			winBox.d_max.d_y.d_offset -= val;
		}
		
		LOG_DEBUG("Setting image area to: " << CEGUI::PropertyHelper<CEGUI::URect>::toString(winBox));
		imageWin->setProperty("ImageArea", CEGUI::PropertyHelper<CEGUI::URect>::toString(winBox));
	}
	
	imageWin->setProperty("Image", imageName);
}

CEGUI::Sizef MGE::getRespectRatioSize(CEGUI::Sizef reqSize, const CEGUI::Sizef& orgSize) {
	float orgRatio = orgSize.d_width / orgSize.d_height;
	float reqRatio = reqSize.d_width / reqSize.d_height;
	
	LOG_DEBUG("reqSize=" << reqSize << " orgSize=" << orgSize << " => orgRatio=" << orgRatio << "reqRatio=" << reqRatio);
	
	if (reqSize.d_width == 0) {
		reqSize.d_width  = reqSize.d_height * orgRatio;
	} else if (reqSize.d_height == 0 || orgRatio > reqRatio) {
		reqSize.d_height = reqSize.d_width  / orgRatio;
	} else /*if (orgRatio < reqRatio)*/ {
		reqSize.d_width  = reqSize.d_height * orgRatio;
	}
	
	LOG_DEBUG("newSize=" << reqSize);
	
	return reqSize;
}
