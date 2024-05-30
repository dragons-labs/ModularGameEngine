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

#include "gui/modules/Gui3DProgressBar.h"
#include "LogSystem.h"

MGE::ProgressBar3D::ProgressBar3D(Ogre::SceneNode* _parent, const Ogre::String& _nodeName, const Ogre::Vector3& _offset, bool _isNotMovable) :
	MGE::GUI3D(_parent, _nodeName, 3.0, 0.4, _offset)
{
	LOG_DEBUG("create ProgressBar3D: " << _nodeName);
	
	setGUI(128, 16, false);
	
	bgWin = static_cast<CEGUI::ProgressBar*>(
		CEGUI::WindowManager::getSingleton().loadLayoutFromFile("ProgressBar3D.layout")
	);
	pbWin = static_cast<CEGUI::ProgressBar*>( bgWin->getChild("pb") );
	guiOnTexture->getRootWindow()->addChild(bgWin);
}

MGE::ProgressBar3D::~ProgressBar3D() {
	LOG_DEBUG("destroy ProgressBar3D: " << billboardSet->getName());
	CEGUI::WindowManager::getSingleton().destroyWindow(bgWin);
}

void MGE::ProgressBar3D::setProgress(float progress, CEGUI::argb_t newColourARGB) {
	LOG_DEBUG("set progress on: " << billboardSet->getName() << " to value: " << progress);
	setProgress(progress);
	if (colourARGB != newColourARGB) {
		colourARGB  = newColourARGB;
		pbWin->setProperty(
			"ProgressColour",
			CEGUI::PropertyHelper<CEGUI::Colour>::toString(CEGUI::Colour(colourARGB))
		);
	}
}
