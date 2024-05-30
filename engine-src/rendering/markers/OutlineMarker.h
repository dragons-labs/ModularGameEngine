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

#pragma   once
#include "config.h"

#include "rendering/markers/VisualMarkers.h"

namespace MGE {

/// @addtogroup VisualMarkers
/// @{
/// @file

/**
 * @brief outline marker based on rescaled cone of marked node and stencil buffer operations
 *
 * use stencil passes in compostitor, see resources/Ogre/Compositor/workspaces.compositor
 */
class OutlineVisualMarker : public MGE::VisualMarker {
public:
	/**
	 * @brief constructor
	 * 
	 * @param material        material (colour) of outline
	 * @param mode            type of MGE::VisualMarker
	 * @param linesThickness  thickness of outline lines (relative to object size)
	 * @param node            scene node to attach box
	 */
	OutlineVisualMarker(const Ogre::String& material, int mode, float linesThickness, Ogre::SceneNode* node);
	
	/// destructor
	~OutlineVisualMarker();
	
	/**
	 * @brief prepare renderable based on ABB info from object
	 * 
	 * @param aabb  axis aligned bounding box from object (in LOCAL object space)
	 */
	void setupVertices(const Ogre::AxisAlignedBox& aabb) override {};
	
	/**
	 * @brief updating colour and thickness of outline
	 * 
	 * @param _               unused (for API compatibility)
	 * @param markerMaterial  material to set on marker
	 * @param linesThickness  thickness of outline lines (relative to object size)
	 */
	void update(int _, const Ogre::String& markerMaterial, float linesThickness) override;
	
	/**
	 * @brief return box movable object
	 */ 
	Ogre::MovableObject* getMovable() override {
		return NULL;
	}
	
protected:
	/// pointer to scene node with clone of marked object
	Ogre::SceneNode* stencilGlowNode;
	
	/// helper function for recursive create stencil glow scene node (@a dst) by cloning @a src
	/// set material and modyfiy render group
	void recursiveCreateStencilGlowNode(Ogre::SceneNode* src, Ogre::SceneNode* dst, const Ogre::String& material);
	
	/// helper function for recursive update material on stencil glow scene node
	void recursiveSetMaterial(Ogre::SceneNode* node, const Ogre::String& material);
	
	/// helper function for recursive reset render group of base node
	void recursiveCleanStencilGlow(Ogre::SceneNode* node);
};

/// @}

}
