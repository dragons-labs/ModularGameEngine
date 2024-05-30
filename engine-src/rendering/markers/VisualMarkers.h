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

#include "force_inline.h"
#include "BaseClasses.h"

#include <Ogre.h>
#include <string_view>
namespace pugi { class xml_node; }

namespace MGE {

/// @addtogroup VisualMarkers
/// @{
/// @file

/**
 * @brief Visual Marker base class
 */
class VisualMarker {
public:
	/// primary type of VisualMarker
	enum PrimaryTypes {
		/// use oriented bounding box renderable (MGE::OBBoxRenderable) as marker
		OBBOX   = 0x000,
		/// use plane / billboard with texture as marker
		PLANE   = 0x100,
		/// use decal with texture as marker
		DECAL   = 0x200,
		/// use object outline (stencil glow) with colour as marker
		OUTLINE = 0x300,
		/// mask for select PrimaryTypes value from markerType value
		PrimaryTypeMask = 0xf00
	};
	
	/// line thickness type
	enum LineThicknessTypes {
		/// no line thickness
		NO_THICKNESS = 0x00,
		/// absolute line thickness (in world size units)
		ABSOLUTE_THICKNESS = 0x10,
		/// line thickness relative to minimum box size
		BOX_PROPORTIONAL_THICKNESS = 0x20,
		/// mask for select LineThicknessTypes value from markerType value
		LineThicknessTypeMask = 0xf0
	};
	
	/// subtype of oriented bounding box renderable VisualMarker
	enum OOBoxSubTypes {
		/// draw full edges of oriented bounding box
		FULL_BOX   = 0x0,
		/// draw only corners markers of oriented bounding box
		CORNER_BOX = 0x1,
		/// mask for select OOBoxSubTypes value from markerType value
		OOBoxSubTypeMask = 0xf
	};
	
	/**
	 * @brief convert string notation of PrimaryTypes, LineThicknessTypes and OOBoxSubTypes to numeric value (single flag value)
	 * 
	 * @param s string to convert
	 */
	inline static uint16_t stringToTypes(const std::string_view& s) {
		if (s == "OBBOX")                            return OBBOX;
		else if (s == "PLANE")                       return PLANE;
		else if (s == "DECAL")                       return DECAL;
		else if (s == "OUTLINE")                     return OUTLINE;
		else if (s == "NO_THICKNESS")                return NO_THICKNESS;
		else if (s == "ABSOLUTE_THICKNESS")          return ABSOLUTE_THICKNESS;
		else if (s == "BOX_PROPORTIONAL_THICKNESS")  return BOX_PROPORTIONAL_THICKNESS;
		else if (s == "FULL_BOX")                    return FULL_BOX;
		else if (s == "CORNER_BOX")                  return CORNER_BOX;
		return 0;
	}
	
	/**
	 * @brief prepare renderable based on ABB info from object
	 * 
	 * @param aabb  axis aligned bounding box from object (in LOCAL object space)
	 */
	virtual void setupVertices(const Ogre::AxisAlignedBox& aabb) = 0;
	
	/**
	 * @brief updating type (mode) and colour of box
	 * 
	 * @param markerType      type of marker
	 * @param markerMaterial  material to set on marker
	 *                        (for decal markers - name of decals textures group - emissive, diffuse and normal)
	 * @param linesThickness  thickness of box lines (used only for some markerType values)
	 */
	virtual void update(int markerType, const Ogre::String& markerMaterial, float linesThickness) = 0;
	
	/**
	 * @brief return box movable object
	 */ 
	virtual Ogre::MovableObject* getMovable() = 0;
	
	// constructor
	VisualMarker(int markerType) :
		type(markerType)
	{}
	
	/// destructor
	virtual ~VisualMarker() {}
	
protected:
	friend class VisualMarkersManager;
	int type;
};

/**
 * @brief Visual Marker Settings
 */
struct VisualMarkerSettingsSet {
	/**
	 * @brief constructor
	 * 
	 * @param _markerType      type of marker (see VisualMarker for details)
	 * @param _materialName    material to marker for VisualMarker (eg. simple color material from MGE::OgreUtils::getColorDatablock)
	 *                         (for decal markers - name of decals textures group - emissive, diffuse and normal)
	 * @param _linesThickness  thickness of box lines (used only for some markerType values)
	 *                         scale factor for decal markers
	 */
	VisualMarkerSettingsSet(int _markerType = 0, const Ogre::String& _materialName = Ogre::BLANKSTRING, float _linesThickness = 0) :
		markerType(_markerType),
		materialName(_materialName),
		linesThickness(_linesThickness)
	{}
	
	/**
	 * @brief set values from XML
	 * 
	 * @param xmlArchive xml archive object, with pointer to xml node, that will be using for load state of this object
	 */
	void loadFromXML(const pugi::xml_node& xmlNode);
	
	/// type of box (see VisualMarker for details)
	int          markerType;
	
	/// material to set for VisualMarker
	Ogre::String materialName;
	
	/// thickness of box lines (used only for some markerType values)
	float        linesThickness;
};

/**
 * @brief Visual Marker Manager
 */
class VisualMarkersManager :
	public MGE::TrivialSingleton<MGE::VisualMarkersManager>
{
public:
	/**
	 * @brief show Visual Marker
	 * 
	 * @param node            scene node to attach VisualMarker
	 * @param aabb            axis-aligned bounding box to show (when NULL use aabb from @a node)
	 * @param markerType      type of marker (see VisualMarker for details)
	 * @param materialName    material to set for VisualMarker (eg. simple color material from MGE::OgreUtils::getColorDatablock)
	 *                        (for decal markers - name of decals textures group - emissive, diffuse and normal)
	 * @param linesThickness  thickness of box lines (used only for some markerType values)
	 *                        scale factor for decal markers
	 */
	VisualMarker* showMarker(Ogre::SceneNode* node, const Ogre::AxisAlignedBox* aabb, int markerType, const Ogre::String& materialName, float linesThickness);
	
	/**
	 * @brief show Visual Marker
	 * 
	 * @param node            scene node to attach VisualMarker
	 * @param aabb            axis-aligned bounding box to show (when NULL use aabb from @a node)
	 * @param markerSettings  settings set for marker box
	 */
	VisualMarker* showMarker(Ogre::SceneNode* node, const Ogre::AxisAlignedBox* aabb, const VisualMarkerSettingsSet& markerSettings) {
		return showMarker(node, aabb, markerSettings.markerType, markerSettings.materialName, markerSettings.linesThickness);
	}
	
	/**
	 * @brief hide (remove) Visual Marker from @a node
	 */
	void hideMarker(Ogre::SceneNode* node);
	
	/**
	 * @brief return Visual Marker from @a node
	 */
	VisualMarker* getMarker(Ogre::SceneNode* node) {
		auto iter = markers.find( node );
		if (iter != markers.end()) {
			return iter->second;
		} else {
			return NULL;
		}
	}
	
protected:
	/// map of objects with boxes
	std::map<Ogre::SceneNode*, VisualMarker*> markers;
	
protected:
	friend class TrivialSingleton;
	VisualMarkersManager()  = default;
	~VisualMarkersManager() = default;
};

FORCE_INLINE constexpr int operator| (VisualMarker::PrimaryTypes a, VisualMarker::LineThicknessTypes b) { return static_cast<int>(a) | static_cast<int>(b); }
FORCE_INLINE constexpr int operator| (VisualMarker::LineThicknessTypes a, VisualMarker::PrimaryTypes b) { return static_cast<int>(a) | static_cast<int>(b); }

FORCE_INLINE constexpr int operator| (VisualMarker::PrimaryTypes a, VisualMarker::OOBoxSubTypes b) { return static_cast<int>(a) | static_cast<int>(b); }
FORCE_INLINE constexpr int operator| (VisualMarker::OOBoxSubTypes a, VisualMarker::PrimaryTypes b) { return static_cast<int>(a) | static_cast<int>(b); }

FORCE_INLINE constexpr int operator| (VisualMarker::OOBoxSubTypes a, VisualMarker::LineThicknessTypes b) { return static_cast<int>(a) | static_cast<int>(b); }
FORCE_INLINE constexpr int operator| (VisualMarker::LineThicknessTypes a, VisualMarker::OOBoxSubTypes b) { return static_cast<int>(a) | static_cast<int>(b); }

/// @}

}
