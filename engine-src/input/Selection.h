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

#include "config.h"

#include "BaseClasses.h"
#include "ModuleBase.h"
#include "input/InputSystem.h"
#include "physics/Raycast_forward.h"

#include <OgreVector2.h>
#include <OgreVector3.h>
#include <OgreColourValue.h>
#include <OISMouse.h>
#include <list>

namespace CEGUI { class Window; }

namespace MGE { class CameraNode; }
namespace MGE { class SimpleBox; }
namespace MGE { class SimplePolygonalChain; }
namespace MGE { class SelectionContextMenu; }

namespace MGE {

/// @addtogroup Input
/// @{
/// @file

/**
 * @brief Object selection class
 */
class Selection :
	public MGE::InputSystem::Listener,
	public MGE::Module,
	public MGE::Singleton<Selection>
{
public:
	/// base class for "selected object set"
	struct SelectionSetBase {
		/**
		 * @brief callback function for selecting/deselecting group of object
		 * 
		 * @param searchResults     results of object serching (set of object to operate)
		 * @param _selectSwitchMode operation mode to perform on objects (select, switch, etc), see @ref SelectionSwitchModes
		 * @param _selectionMode    mode of selection operation, see @ref SelectionModes
		 */
		virtual bool select(MGE::RayCast::ResultsPtr searchResults, int _selectSwitchMode, int _selectionMode) = 0;
		
		/**
		 * @brief return query mask for reduce searching to specific object types, see @ref MGE::QueryFlags
		 */
		virtual int getSearchMask() = 0;
		
		/// destructor
		virtual ~SelectionSetBase() {}
	};
	
	/// modes which can operate selection tool 
	enum SelectionModes {
		/// selection disable
		NONE = 0,
		/// select objects / actors using @ref selectedObjectsPtr
		GET_OBJECTS          = 1 << 0,
		/// select rectangle area
		GET_RECTANGLE        = 1 << 1,
		/// select polygonal chain
		GET_POLYGONAL_CHAIN  = 1 << 2,
		/// results is from single ray (only for select() callback in @ref SelectionSetBase)
		FROM_POINT           = 1 << 8,
		/// results is from multiple ray (only for select() callback in @ref SelectionSetBase)
		FROM_AREA            = 1 << 9,
	};
	
	/// modes which can operate selection tool 
	enum SelectionSwitchModes {
		/// reset selection and add new found object to selection set
		RESET_SELECTION,
		/// add  new found object to selection set
		ADD_TO_SELECTION,
		/// remove new found object from selection set
		REMOVE_FROM_SELECTION,
		/// switch selection for new found object
		SWITCH_SELECTION
	};
	
	
	/**
	 * @brief set selection mode
	 * 
	 * @param[in]     mode       selection mode, see @ref SelectionModes
	 * @param[out]    objects    pointer to std::set to put selected objects
	 * @param[out]    points     pointer to std::list to put selected points
	 * @param[in]     precision  the squared distance betwen start and end point in polygonal chain below which we recognize it as closed chain
	 */
	void setSelectionMode(
		int                       mode      = NONE,
		SelectionSetBase*         objects   = NULL,
		std::list<Ogre::Vector3>* points    = NULL,
		float                     precision = 0.25
	);
	
	/**
	 * @brief set context menu (for right click selection) implementation object
	 */
	inline void setContextMenu(MGE::SelectionContextMenu* menu) {
		contextMenu = menu;
	}
	
	/**
	 * @brief return current selection mode
	 */
	int getSelectionMode() {
		return selectionMode;
	}
	
	/** 
	 * @name input interface
	 * 
	 * @{
	 */
		/**
		 * @brief select object, init multiple-object selection or finish special mode
		 * 
		 * @param[in] mouseViewportPos  mouse click position relative to @a window
		 * @param[in] clickButtonID     OIS mouse button id
		 * @param[in] window            pointer to GUI window used to make selection (NULL if use main (non-gui) window)
		 */
		virtual bool mousePressed(const Ogre::Vector2& mouseViewportPos, OIS::MouseButtonID clickButtonID, const OIS::MouseEvent& arg, MGE::InteractiveTexture*& _activeTextureObject, CEGUI::Window* window) override;
		
		/**
		 * @brief update multiple-object selection 
		 * 
		 * @param[in] mouseViewportPos  mouse click position relative to current selection viewport
		 * 
		 * @return
		 *   @li false - when when the selection was not initialized
		 *   @li true  - when the selection box has been updated
		 */
		virtual bool mouseMoved(const Ogre::Vector2& mouseViewportPos, const OIS::MouseEvent& arg, MGE::InteractiveTexture* _activeTextureObject) override;
		
		/**
		 * @brief finish multiple-object selection
		 * 
		 * @param[in] clickButtonID     OIS mouse button id
		 * @param[in] mouseViewportPos  mouse click position relative to current selection viewport
		 * 
		 * @return
		 *   @li false - when when the selection was not initialized
		 *   @li true  - when the selection has been finalized
		 */
		virtual bool mouseReleased(const Ogre::Vector2& mouseViewportPos, OIS::MouseButtonID clickButtonID, const OIS::MouseEvent& arg, MGE::InteractiveTexture* _activeTextureObject) override;
	/**
	 * @}
	 */
	
	/**
	 * @brief constructor - set selection mode based on XML configuration
	 * 
	 * @param[in] xmlNode           XML configuration node
	 */
	Selection(const pugi::xml_node& xmlNode);
	
protected:
	/**
	 * @brief constructor - set selection mode
	 * 
	 * @param[in]     _selectionBoxColour           colour for selection box
	 * @param[in]     _selectionBoxLineThickness    lines thickness for selection box (0 for one pixel default)
	 * @param[in]     _selectionChainColour         colour for selection polygonal chain
	 * @param[in]     _selectionChainLineThickness  lines thickness for selection polygonal chain (0 for one pixel default)
	 */
	Selection(
		const Ogre::ColourValue& _selectionBoxColour,   float _selectionBoxLineThickness,
		const Ogre::ColourValue& _selectionChainColour, float _selectionChainLineThickness
	);
	
	/// destructor
	~Selection();
	
	/** 
	 * @name input interface (variables)
	 * 
	 * @{
	 */
		/// mouse position when pressed 
		Ogre::Vector2   clickMousePos;
		
		/// window which occurred mouse pressed
		CEGUI::Window*  clickWindow;
		
		/// true when we have mouse pressed
		/// @note when false @ref clickMousePos and @ref clickWindow are invalise
		bool            clickHasDown;
	/**
	 * @}
	 *
	 * 
	 * @name GUI element used for selecting
	 * 
	 * @{
	 */
		/**
		 * @brief (re)init selection box for specified point, window and camera
		 * 
		 * @param[in] x        x coordinate of point
		 * @param[in] y        y coordinate of point
		 * @param[in] camera   pointer to camera to determinate viewport for drawing selection box
		 */
		void reInitSelectionBox(float x, float y, MGE::CameraNode* camera);
		
		/// (re)init polygonal chain
		void reInitPolygonalChainMarker(MGE::CameraNode* camera);
		
		/**
		 * @brief finish selection at current mouse position and return 4 rays set and destroy selection box
		 * 
		 * @param[out] rays          vector of rays to add 4 rays set related to selection rectangular corners:
		 *                           left-top, right-top, left-bottom, right-bottom
		 * @param[in]  selectionStop point of finish selection
		 * @param[in]  camera        pointer to camera to determinate viewport for drawing selection box
		 */
		void finishSelection(std::vector<Ogre::Ray>& rays, const Ogre::Vector2& selectionStop, MGE::CameraNode* camera);
		
		/// destroy selection box
		void deleteSelectionBox();
		
		/// destroy polygonal chain
		void deletePolygonalChainMarker();
	/**
	 * @}
	 *
	 * 
	 * @name GUI element used for selecting (variables)
	 * 
	 * @{
	 */
		/// pointer to selection box
		MGE::SimpleBox*              selectionBox;
		
		/// point of first mouse position when making of selecting box
		Ogre::Vector2                selectionStart;
		
		/// Camera used to draw selecting box
		MGE::CameraNode*             selectionCamera;
		
		/// pointer to polygonal chain (used for getting list of points)
		MGE::SimplePolygonalChain*   polygonalChainMarker;
		
		/// precision setting for closing polygonal chain
		/// @note when distance betwen BEGIN and X is less than @a polygonalChainPrecision then X = BEGIN and finish getting area type polygonal chain
		float                        polygonalChainPrecision;
		
		/// Context menu to show on (right click selection)
		MGE::SelectionContextMenu*   contextMenu;
	/**
	 * @}
	 *
	 * 
	 * @name selection mode and pointers to put-results objects (variables)
	 * 
	 * @{
	 */
		/// mode of selection operation, see @ref SelectionModes
		int                          selectionMode;
		
		/// pointer to SelectionSet object to put results in object getting modes
		SelectionSetBase*            selectedObjectsPtr;
		
		/// pointer to list of 3D points (Ogre::Vector3) to put results in point getting modes
		std::list<Ogre::Vector3>*    selectedPointsPtr;
	/**
	 * @}
	 *
	 * 
	 * @name selection markers settings
	 * 
	 * @{
	 */
		const Ogre::ColourValue      selectionBoxColour;
		float                        selectionBoxLineThickness;
		const Ogre::ColourValue      selectionChainColour;
		float                        selectionChainLineThickness;
	/**
	 * @}
	 */
};

/// @}

}
