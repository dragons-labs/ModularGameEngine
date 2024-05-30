/*
Copyright (c) 2018-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>

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

#include "rendering/markers/ProjectiveDecalsMarker.h"

#include "LogSystem.h"
#include "rendering/utils/Decals.h"

#include <OgreDecal.h>

MGE::ProjectiveDecalsMarker::ProjectiveDecalsMarker(const Ogre::String& material, int _, float factor, const Ogre::AxisAlignedBox& aabb, Ogre::SceneNode* node) : 
	MGE::VisualMarker(DECAL),
	scale(factor)
{
	LOG_DEBUG("Create ProjectiveDecalsMarker with: " << material);
	
	Ogre::TextureGpu*   textureEmissive = MGE::Decals::getPtr()->getEmissive(material);
	Ogre::TextureGpu*   textureDiffuse  = MGE::Decals::getPtr()->getDiffuse(material);
	Ogre::TextureGpu*   textureNormals   = MGE::Decals::getPtr()->getNormals(material);
	
	Ogre::SceneManager* scnMgr = node->getCreator();
	
	decal = scnMgr->createDecal();
	if (textureEmissive)
		decal->setEmissiveTexture( textureEmissive );
	if (textureDiffuse)
		decal->setDiffuseTexture( textureDiffuse );
	if (textureNormals)
		decal->setNormalTexture( textureNormals );
	
	decalNode = node->createChildSceneNode();
	decalNode->attachObject(decal);
	
	if (textureEmissive)
		scnMgr->setDecalsEmissive( textureEmissive );
	if (textureDiffuse)
		scnMgr->setDecalsDiffuse( textureDiffuse );
	if (textureNormals)
		scnMgr->setDecalsNormals( textureNormals );
	setupVertices(aabb);
}

void MGE::ProjectiveDecalsMarker::setupVertices(const Ogre::AxisAlignedBox& aabb) {
	Ogre::Vector3 min = aabb.getMinimum();
	Ogre::Vector3 max = aabb.getMaximum();
	
	size = max - min;
	Ogre::Vector3 center(
		min.x + size.x * 0.5 ,
		size.y * - 0.9,
		min.z + size.z * 0.5
	);
	size.y = size.y * 2;
	
	decalNode->setPosition( center );
	decalNode->setScale(size * scale); // decal size: (x,z) = projection plane size, y = projection range
}

void MGE::ProjectiveDecalsMarker::update(int, const Ogre::String& material, float factor) {
	Ogre::SceneManager* scnMgr = decalNode->getCreator();
	Ogre::TextureGpu* texture;
	
	texture = MGE::Decals::getPtr()->getEmissive(material);
	if (texture) {
		decal->setEmissiveTexture( texture );
		scnMgr->setDecalsEmissive( texture );
	}
	
	texture = MGE::Decals::getPtr()->getDiffuse(material);
	if (texture) {
		decal->setDiffuseTexture( texture );
		scnMgr->setDecalsDiffuse( texture );
	}
	
	texture = MGE::Decals::getPtr()->getNormals(material);
	if (texture) {
		decal->setNormalTexture( texture );
		scnMgr->setDecalsNormals( texture );
	}
	
	scale = factor;
	decalNode->setScale(size * scale);
}

MGE::ProjectiveDecalsMarker::~ProjectiveDecalsMarker() {
	Ogre::SceneManager* scnMgr = decalNode->getCreator();
	
	decalNode->detachObject(decal);
	scnMgr->destroyDecal(decal);
	decalNode->getParent()->removeChild(decalNode);
	scnMgr->destroySceneNode(decalNode);
}
