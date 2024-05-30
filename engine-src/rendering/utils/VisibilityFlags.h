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

#include <stdint.h>
#include <OgreVisibilityFlags.h>

#include "StringUtils.h"

namespace MGE {

/// @addtogroup Rendering
/// @{
/// @file

/**
 * @brief Visibility flags namespace enumeration.
 * 
 * Visibility flag can be set via setVisibilityFlags(), object is visible when objectFlags & viewportMask != 0
 * default value of flag (if not set) is 0xffffffff, default value of mask is 0x00ff0f0f
 * 
 * `#include <visibilityFlags.h>`
 */
namespace VisibilityFlags {
	/// flag for standard scene elements (default visible)
	const uint32_t OBJECTS   = 1 << 0;
	/// flag for default visible 3D GUI elements (eg. progrssbar)
	const uint32_t GUI_3D    = 1 << 1;
	/// flag for default visible 3D UI element (eg. selection markers)
	const uint32_t UI_3D     = 1 << 2;
	
	/// flag for default invisible objects 3D UI element
	const uint32_t SELECTION = 1 << 4;
	/// flag for default invisible triggers objects
	const uint32_t TRIGGERS  = 1 << 5;
	/// flag for default invisible targets objects
	const uint32_t TARGETS   = 1 << 6;
	
	/// @{
	/// @name using full Ogre::VisibilityFlags namespace
	using namespace Ogre::VisibilityFlags;
		#ifdef __DOCUMENTATION_GENERATOR__
		/// when this is bit is clear, the obj is not rendered at all (bit 30), see Ogre::VisibilityFlags
		const uint32_t LAYER_VISIBILITY           = Ogre::VisibilityFlags::LAYER_VISIBILITY;
		/// object casts shadows - is rendered to shadow camera (bit 31), see Ogre::VisibilityFlags
		const uint32_t LAYER_SHADOW_CASTER        = Ogre::VisibilityFlags::LAYER_SHADOW_CASTER;
		/// reserved - bits 30 and 31 are reserved for Ogre internal use, see Ogre::VisibilityFlags
		const uint32_t RESERVED_VISIBILITY_FLAGS  = Ogre::VisibilityFlags::RESERVED_VISIBILITY_FLAGS;
		#endif
	/// @}
	
	/// mask for viewport modes
	const uint32_t DEFAULT_MASK         = 0x00ff0f0f;
	
	/// convert string to VisibilityFlags value
	inline uint32_t fromString(const std::string_view& s) {
		if (s == "OBJECTS") {
			return OBJECTS;
		} else if (s == "GUI_3D") {
			return GUI_3D;
		} else if (s == "UI_3D") {
			return UI_3D;
		} else if (s == "SELECTION") {
			return SELECTION;
		} else if (s == "TRIGGERS") {
			return TRIGGERS;
		} else if (s == "TARGETS") {
			return TARGETS;
		} else {
			return MGE::StringUtils::toNumeric<uint32_t>(s);
		}
	}
}

/// @}

}
