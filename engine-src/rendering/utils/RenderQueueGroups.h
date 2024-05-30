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

#include "StringUtils.h"

namespace MGE {

/**
 * @brief Ogre render queue groups (higher number => later rendering, but rendering order itself not determined
 *        covering of visible object, importat are material settings e.g. "depth_check" in HLMS)
 *
 * @note V1 and V2 object must be in separated queues -> see setRenderQueueMode()
 *
 * `#include <renderQueueGroups.h>`
 */
namespace RenderQueueGroups {
	enum RenderQueueGroups : uint8_t {
		/// queue for backround V2 objects
		BACKGROUND_V2 = 0,
		/// default queue for backround V1 FAST objects
		BACKGROUND_V1 = 1,
		/// default queue for Ogre V2 objects
		DEFAULT_OBJECTS_V2 = 4,
		/// default queue for Ogre V1 FAST objects
		DEFAULT_OBJECTS_V1 = 5,
		/// queue for stencil glow object (mark as hole in STENCIL_GLOW_OUTLINE queue) for V2 objects
		STENCIL_GLOW_OBJECT_V2 = 8,
		/// queue for stencil glow object (mark as hole in STENCIL_GLOW_OUTLINE queue) for V1 FAST objects
		STENCIL_GLOW_OBJECT_V1 = 9,
		/// queue for stencil glow outline (rendered only when no object in STENCIL_GLOW_OBJECT queues in this place) for V2 objects
		STENCIL_GLOW_OUTLINE_V2 = 10,
		/// queue for stencil glow outline (rendered only when no object in STENCIL_GLOW_OBJECT queues in this place) for V1 FAST objects
		STENCIL_GLOW_OUTLINE_V1 = 11,
		/// queue for 3D GUI V2 objects
		GUI_3D_V2  = 12,
		/// queue for 3D GUI V1 FAST objects
		GUI_3D_V1  = 13,
		/// queue for 3D UI V2 objects
		UI_3D_V2   = 14,
		/// queue for 3D UI V1 FAST objects
		UI_3D_V1   = 15,
		/// queue for overlay (direct on screen rendering) V2 objects
		OVERLAY_V2 = 16,
		/// queue for overlay (direct on screen rendering) V1 FAST objects
		OVERLAY_V1 = 17,
		// max used render queue id + 1
		STOP_RENDER_QUEUE = 18
	};
	
	/// convert string to RenderQueueGroups value
	inline uint8_t fromString(const std::string_view& s, bool isV1 = false) {
		if (s == "OVERLAY") {
			return isV1 ? OVERLAY_V1 : OVERLAY_V2;
		} else if (s == "UI3D") {
			return isV1 ? UI_3D_V1 : UI_3D_V2;
		} else if (s == "GUI3D") {
			return isV1 ? GUI_3D_V1 : GUI_3D_V2;
		} else if (s == "DEFAULT") {
			return isV1 ? DEFAULT_OBJECTS_V1 : DEFAULT_OBJECTS_V2;
		} else if (s == "BACKGROUND") {
			return isV1 ? BACKGROUND_V1 : BACKGROUND_V2;
		} else {
			return MGE::StringUtils::toNumeric<uint8_t>(s);
		}
	}
}

}
