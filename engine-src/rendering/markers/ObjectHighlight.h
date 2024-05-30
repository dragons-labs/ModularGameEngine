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
#include "config.h"

#include "BaseClasses.h"

namespace Ogre {
	class SceneNode;
	class ColourValue;
	class Renderable;
	class HlmsDatablock;
}

#include <map>

namespace MGE {

/// @addtogroup VisualMarkers
/// @{
/// @file

/**
 * @brief Object Highlight Manager
 */
class ObjHighlightManager :
	public MGE::TrivialSingleton<MGE::ObjHighlightManager>
{
public:
	/**
	 * @brief set highlight of @a obj with selected color
	 * 
	 * @param node   scene node with object to highlight
	 * @param color  color of highlight to set
	 *
	 * @note
	 *  color components can be great that 1.0 for more lighter highlight
	 */
	void enable(Ogre::SceneNode* node, const Ogre::ColourValue& color);
	
	/**
	 * @brief unset highlight of @a node
	 * 
	 * @param node   scene node with highlighted object
	 */
	void disable(Ogre::SceneNode* node);
	
protected:
	/// struct for store info about highlight object
	struct HighlightObject {
		/// map of object subitems and its oryginal datablocks
		std::map<Ogre::Renderable*, Ogre::HlmsDatablock*> orgDatablocks;
	};
	
	/// map of objects with highlight
	std::map<Ogre::SceneNode*, HighlightObject*> highlightObjects;
	
	/// helper function for recursive modyfy materials
	void enable(Ogre::SceneNode* node, const Ogre::ColourValue& color, HighlightObject* hObj);
	
	/// helper function for clone and modyfiy material datablock
	Ogre::HlmsDatablock* replaceDatablock(HighlightObject* hObj, Ogre::Renderable* renderable, const Ogre::ColourValue& color);
	
	friend class TrivialSingleton;
	ObjHighlightManager()  = default;
	~ObjHighlightManager() = default;
};

/// @}

}
