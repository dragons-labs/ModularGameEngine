/*
Copyright (c) 2013-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>

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

#include "MainLoopListener.h"
#include "ModuleBase.h"

#include "gui/GuiSystem.h"
#include "gui/GuiGenericWindows.h"

#include <OgreVector2.h>

namespace MGE {

/// @addtogroup Modules
/// @{
/// @file

/**
 * @brief Window with 2D minimap of mission scene
 */
class MiniMap :
	public MGE::GenericWindows::BaseWindowOwner,
	public MGE::Module,
	public MGE::MainLoopListener,
	public MGE::Unloadable,
	public MGE::Singleton<MiniMap>
{
public:
	/// @copydoc MGE::MainLoopListener::update
	/// (used to update units positons on overlayTexture)
	bool update(float gameTimeStep, float realTimeStep) override;
	
	/// @copydoc MGE::GenericWindows::BaseWindowOwner::show
	void show(const CEGUI::String& name = CEGUI::String::GetEmpty()) override;
	
	/// base class for minimap objects info providers
	struct ObjectsInfoProvider {
		/**
		 * @brief init list iterator
		 */
		virtual void resetMinimapInfo() = 0;
		
		/**
		 * @brief return (via reference arguments) minimap symbol (icon) and world positon of (next) object
		 * 
		 * @param[out] buf      pointer to 4bit per channel ARGB buffer with symbol
		 *                        (NULL when object should not show on minimap)
		 * @param[out] width    width  of symbol (line length in buf)
		 * @param[out] height   height of symbol (number of line in buf)
		 * @param[out] height   height of symbol (number of line in buf)
		 * @param[out] worldPos world 3D position
		 * 
		 * @return return false when no object to get (end iteration)
		 */
		virtual bool getNextMinimapInfo(const uint16_t*& buf, int& width, int& height, Ogre::Vector3& worldPos) = 0;
		
		/// destructor
		virtual ~ObjectsInfoProvider() {}
	};
	
	/**
	 * @brief set pointer to objects info provider
	 */
	void setObjectInfoProvider(ObjectsInfoProvider* p) {
		objectsInfoProvider = p;
	}
	
	/// constructor
	/**
	 * @brief constructor - initialise and show world map
	 * 
	 * @param[in] baseWin                     pointer to parent (tabs, frame, etc) window object
	 * @param[in] image                       minimap (background) image 
	 * @param[in] imageGroup                  resources group for minimap (background) image 
	 * @param[in] upperLeftCornerPositionIn3D upper left corner of mini map position in game 3D world
	 * @param[in] sizeIn3D                    size of mini map in game 3D world units (offset from upperLeftCorner to lowerRightCorner)
	 */
	MiniMap(
		MGE::GenericWindows::BaseWindow* baseWin,
		const CEGUI::String& image, const CEGUI::String& imageGroup,
		const Ogre::Vector2& upperLeftCornerPositionIn3D,
		const Ogre::Vector2& sizeIn3D
	);
	
	/**
	 * @brief create MiniMap based on XML configuration
	 * 
	 * @param[in] xmlNode           XML configuration node
	 */
	static MiniMap* create(const pugi::xml_node& xmlNode);
	
	/// destructor
	~MiniMap();
	
protected:
	/**
	 * @brief handle click on minimap
	 * 
	 * @param[in] args - OIS Event detail/description
	 */
	bool handleClick(const CEGUI::EventArgs& args);
	
	/**
	 * @brief convert minimap coordinates to 3D world coordinates
	 * 
	 * @param[in] pos - minimap position
	 */
	Ogre::Vector3 minimapToWorld(const glm::vec2& pos);
	
	/**
	 * @brief convert 3D coordinates to minimap coordinates
	 * 
	 * @param[in] pos - game 3D world position
	 */
	Ogre::Vector2 worldToOverlayMinimap(const Ogre::Vector3& pos);
	
	/**
	 * @brief put square point marker on minimap overlay buffer
	 * 
	 * @param[in] buf         pointer to buffer
	 * @param[in] bufWidth    width (x size) of buffer
	 * @param[in] bufHeight   height (y size) of buffer
	 * @param[in] x           x buffer coordinate of point
	 * @param[in] y           y buffer coordinate of point
	 * @param[in] size        length of side of the square
	 * @param[in] argb_color  color of the point marker
	 */
	static void putPoint(uint16_t* buf, int bufWidth, int bufHeight, int x, int y, uint8_t size, uint16_t argb_color);
	
	/**
	 * @brief put cross (+) point marker on minimap overlay buffer
	 * 
	 * @param[in] buf         pointer to buffer
	 * @param[in] bufWidth    width (x size) of buffer
	 * @param[in] bufHeight   height (y size) of buffer
	 * @param[in] x           x buffer coordinate of point
	 * @param[in] y           y buffer coordinate of point
	 * @param[in] size        length of side of cross arm
	 * @param[in] argb_color  color of the point marker
	 */
	static void putCross(uint16_t* buf, int bufWidth, int bufHeight, int x, int y, uint8_t size, uint16_t argb_color);

private:
	Ogre::Vector2         miniMapSizeIn3D;
	float                 toOverlayMiniMapScaleY;
	float                 toOverlayMiniMapScaleX;
	float                 fromMiniMapScaleY;
	float                 fromMiniMapScaleX;
	float                 miniMapOffsetY;
	float                 miniMapOffsetX;
	
	CEGUI::Texture*       overlayTexture;
	CEGUI::Window*        minimap;
	
	static const int      overlayScale;
	CEGUI::Sizef          overlayTextureSize;
	int                   overlayTextureBufferSize;
	
	bool                  isVisible;
	
	ObjectsInfoProvider*  objectsInfoProvider;
	
	void recalculateScale();
	bool handleSized(const CEGUI::EventArgs& args);
	
	bool onShow(const CEGUI::EventArgs& args);
	bool onHide(const CEGUI::EventArgs& args);
	
};

/// @}

}
