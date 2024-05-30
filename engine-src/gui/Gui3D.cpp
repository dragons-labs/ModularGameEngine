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

#include "gui/Gui3D.h"

#include "LogSystem.h"
#include "rendering/utils/RenderQueueGroups.h"

#include <OgreBillboard.h>

MGE::GUI3D::GUI3D(
	Ogre::SceneNode* _parent,
	const Ogre::String& _name,
	Ogre::Real _width,
	Ogre::Real _height,
	const Ogre::Vector3& _offset,
	bool _inWorldSpace,
	bool _autoOrientation,
	const Ogre::Quaternion& _orientation
) :
	guiOnTexture(NULL)
{
	LOG_DEBUG("create billboard for GUI3D: " << _name);
	Ogre::SceneManager* scnMgr = _parent->getCreator();
	
	billboardSet = scnMgr->createBillboardSet(1);
	if (! _autoOrientation) {
		billboardSet->setBillboardType(Ogre::v1::BBT_PERPENDICULAR_COMMON);
		/*
			// create translated child node for set orientation
			// args: const MGE::Utils::String& name, Ogre::SceneNode* parent, Ogre::Vector3 worldTranslate, Ogre::Vector3 localTranslate, Ogre::Quaternion orientation
			Ogre::Vector3 invScale( Ogre::Vector3::UNIT_SCALE / parent->_getDerivedScale() );
			Ogre::SceneNode* node = MGE::NamedSceneNodes::createSceneNode(
				name,
				parent,
				Ogre::SCENE_DYNAMIC,
				parent->_getDerivedOrientation().Inverse() * worldTranslate * invScale + localTranslate,
				orientation,
				invScale
			);
		*/
		// set direction (orthogonal vector to billboard plane)
		billboardSet->setCommonDirection( _orientation.zAxis() );
		// set billboard plane up vector
		billboardSet->setCommonUpVector( _orientation.yAxis() );
	} /* else { // BillboardSet defaults ...
		billboardSet->setBillboardType(Ogre::v1::BBT_POINT);
		billboardSet->setBillboardOrigin(Ogre::v1::BBO_CENTER);
	} */
	
	billboardSet->setBillboardsInWorldSpace(_inWorldSpace);
	billboardSet->setDefaultDimensions(_width, _height);
	billboardSet->setAutoextend(false);
	billboardSet->setRenderQueueGroup(MGE::RenderQueueGroups::GUI_3D_V1);
	
	billboardSet->setName(_name);
	billboardSet->Ogre::Renderable::setDatablock("MAT_MISSING_TEXTURE");
	_parent->attachObject(billboardSet);
	
	billboardSet->createBillboard(_offset);
}

void MGE::GUI3D::setGUI(
	int _resX, int _resY, bool _isInteractive
) {
	LOG_DEBUG("create GUI for GUI3D: " << billboardSet->getName());
	
	delete guiOnTexture;
	guiOnTexture = new MGE::GUIOnTexture(billboardSet->getName(), _resX, _resY, billboardSet->getParentSceneNode()->getCreator(), _isInteractive, false, billboardSet);
}

MGE::GUI3D::~GUI3D() {
	delete guiOnTexture;
	billboardSet->getParentSceneNode()->getCreator()->destroyBillboardSet(billboardSet);
}
