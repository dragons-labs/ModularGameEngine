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
 * @brief marker as plane with texture
 */
class ProjectiveDecalsMarker : public MGE::VisualMarker {
public:
	/**
	 * @brief constructor
	 * 
	 * @param material   base name of textures for decal
	 * @param _          unused (for API compatibility / future usage)
	 * @param factor     scale factor for plane (plane size calculated as circumscribed circle on aabb will be multiplied by this value)
	 * @param aabb       axis aligned bounding box from object (in LOCAL object space)
	 * @param node       scene node to attach box
	 */
	ProjectiveDecalsMarker( const Ogre::String& material, int _, float factor, const Ogre::AxisAlignedBox& aabb, Ogre::SceneNode* node );
	
	/// destructor
	~ProjectiveDecalsMarker();
	
	/**
	 * @brief prepare renderable based on ABB info from object
	 * 
	 * @param aabb  axis aligned bounding box from object (in LOCAL object space)
	 */
	void setupVertices(const Ogre::AxisAlignedBox& aabb) override;
	
	/**
	 * @brief updating colour of outline
	 * 
	 * @param _          unused (for API compatibility)
	 * @param material   material to set on marker
	 * @param factor     scale factor for plane (plane size calculated as circumscribed circle on aabb will be multiplied by this value)
	 */
	void update(int _, const Ogre::String& material, float factor) override;
	
	/**
	 * @brief return box movable object
	 */ 
	Ogre::MovableObject* getMovable() override {
		return NULL;
	}
	
protected:
	/// pointer to scene node with decal (need for scaling)
	Ogre::SceneNode* decalNode;
	
	/// pointer to decal object
	Ogre::Decal* decal;
	
	/// size of abb decal parent
	Ogre::Vector3 size;
	
	/// extra scale factor for decal (how much of decal goes beyond the aabb area)
	Ogre::Real scale;
};

/// @}

}
