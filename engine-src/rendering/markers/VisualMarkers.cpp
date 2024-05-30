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

#include "rendering/markers/VisualMarkers.h"

#include "XmlUtils.h"
#include "data/property/XmlUtils_Ogre.h"

#include "data/utils/OgreUtils.h"
#include "physics/utils/OgreColisionBoundingBox.h"

#include "rendering/utils/VisibilityFlags.h"
#include "rendering/utils/RenderQueueGroups.h"

#include "rendering/markers/OBBoxRenderableImplV2.inl"
#include "rendering/markers/OutlineMarker.h"
#include "rendering/markers/TexturePlaneMarker.h"
#include "rendering/markers/ProjectiveDecalsMarker.h"

MGE::VisualMarker* MGE::VisualMarkersManager::showMarker(Ogre::SceneNode* node, const Ogre::AxisAlignedBox* aabb, int markerType, const Ogre::String& materialName, float linesThickness) {
	int primaryType = (markerType & VisualMarker::PrimaryTypeMask);
	VisualMarker* marker = getMarker(node);
	if (marker) {
		if ((marker->type & VisualMarker::PrimaryTypeMask) == primaryType) {
			marker->update(markerType, materialName, linesThickness);
			return marker;
		}
		delete marker;
	}
	
	Ogre::AxisAlignedBox aabbObj;
	if (!aabb && (primaryType != VisualMarker::OUTLINE) ) {
		// get aabb
		MGE::OgreColisionBoundingBox::getLocalAABB(node, &aabbObj);
		aabb = &aabbObj;
	}
	
	// create VisualMarker
	switch (primaryType) {
		case VisualMarker::OBBOX:
			marker = new MGE::OBBoxRenderable(materialName, markerType, linesThickness, *aabb, node);
			break;
		case VisualMarker::PLANE:
			marker = new MGE::TexturePlaneMarker(materialName, markerType, linesThickness, *aabb, node);
			break;
		case VisualMarker::DECAL:
			marker = new MGE::ProjectiveDecalsMarker(materialName, markerType, linesThickness, *aabb, node);
			break;
		case VisualMarker::OUTLINE:
			marker = new MGE::OutlineVisualMarker(materialName, markerType, linesThickness, node);
			break;
	}
	
	// register and return VisualMarker
	markers[node] = marker;
	return marker;
}

void MGE::VisualMarkersManager::hideMarker(Ogre::SceneNode* node) {
	auto iter = markers.find( node );
	if (iter != markers.end()) {
		delete iter->second;
		markers.erase(iter);
	}
}

/**
@page XMLSyntax_Misc

@subsection XMLNode_VisualMarkerSettingsSet Visual Marker Settings Set

Tag name of tis node depends of usage context. This node is used for create @ref MGE::VisualMarkerSettingsSet struct to configure visual marker. This node have following attributes:
	- @c markerType
	- @c materialName
	- @c linesThickness

For OBBOX and OUTLINE type instead of @c materialName attribute can use @c \<Color\> subnode with @ref XML_ColourValue for simple color material from @ref MGE::OgreUtils::getColorDatablock.

For DECAL type @c materialName is prefix name of texture

@c markerType is combination of zero or one string from follwing strings group:
	- OBBOX PLANE DECAL OUTLINE (see @ref MGE::VisualMarker::PrimaryTypes)
	- NO_THICKNESS ABSOLUTE_THICKNESS BOX_PROPORTIONAL_THICKNESS (see @ref MGE::VisualMarker::LineThicknessTypes)
	- FULL_BOX CORNER_BOX (see @ref MGE::VisualMarker::OOBoxSubTypes)
	.
For example "OBBOX NO_THICKNESS CORNER_BOX".
*/

void MGE::VisualMarkerSettingsSet::loadFromXML(const pugi::xml_node& xmlNode) {
	pugi::xml_attribute xmlAttrib;
	
	linesThickness = xmlNode.attribute("linesThickness").as_float(linesThickness);
	
	if ( (xmlAttrib = xmlNode.attribute("markerType")) ) {
		markerType = MGE::StringUtils::stringToNumericMask<uint16_t>(
			xmlAttrib.as_string(),
			&MGE::VisualMarker::stringToTypes
		);
	}
	
	if ( (xmlAttrib = xmlNode.attribute("materialName")) ) {
		materialName = xmlAttrib.as_string();
	} else {
		pugi::xml_node xmlSubNode = xmlNode.child("Color");
		if (xmlSubNode) {
			materialName = MGE::OgreUtils::getColorDatablock(
				MGE::XMLUtils::getValue<Ogre::ColourValue>(xmlSubNode)
			);
		}
	}
}
