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

#include "StoreRestoreSystem.h"
#include "BaseClasses.h"

#include "modules/editor/AxisGizmo.h"
#include "input/SelectionSet.h"
#include "rendering/markers/VisualMarkers.h"
#include "data/utils/OgreSceneObjectInfo.h"

#include <pugixml.hpp>
#include <CEGUI/CEGUI.h>

namespace MGE {

/// @addtogroup Modules
/// @{
/// @file

/**
 * @brief create window for scene editor
 */
class Editor MGE_CLASS_FINAL :
	public MGE::Module,
	public MGE::Unloadable,
	public MGE::Singleton<MGE::Editor>,
	public MGE::AxisGizmo::Callback
{
public:
	/**
	 * @brief constructor
	 * 
	 * @param  xmlArchive  xml archive object, with pointer to xml node with editor configuration info
	 * @param  scnMgr      pointer to SceneManager used for creating some editor stuff
	 */
	Editor(const pugi::xml_node& xmlNode, Ogre::SceneManager* scnMgr);
	
protected:
	/// destructor
	~Editor();
	
	/**
	 * @brief set @a node name and item info in editor window
	 * 
	 * @param[in] node              scene node to retrieve transformation
	 */
	void setNodeInfo(Ogre::SceneNode* node);
	
	/**
	 * @brief set @a node transformation info in editor window
	 * 
	 * @param[in] node              scene node to retrieve transformation
	 * @param[in] updateInitValues  when true set initial values (for gizmo, offset mode, etc)
	 */
	void setTransformInfo(Ogre::SceneNode* node, bool updateInitValues = true);
	
	/**
	 * @brief get transformation info from editor window
	 * 
	 * @param[out] position         vector to put position info (in PARENT space)
	 * @param[out] orientation      quaternion to put orientation info (in PARENT space)
	 * @param[out] scale            vector to scale position info (in LOCAL space)
	 * @param[in]  node             scene node to get additional info for conversion transform and value spaces (when NULL skip conversion)
	 */
	void getTransformInfo(Ogre::Vector3& position, Ogre::Vector3& scale, Ogre::Quaternion& orientation, Ogre::SceneNode* node = NULL);
	
	/// @copydoc MGE::AxisGizmo::Callback::gizmoCallback
	void gizmoCallback(
		int gizmoMode, Ogre::Node::TransformSpace transformSpace, int axis, Ogre::SceneNode* node,
		const Ogre::Vector2& mouseClickPoint, const Ogre::Vector2& mouseCurrentPoint, const OIS::MouseEvent& mouseArg,
		bool  endOfOperation
	) override;
	
	/**
	 * @brief update nodes
	 * 
	 * @param[in]  position          position in PARENT space (direct writing to XML)
	 * @param[in]  scale             scale in LOCAL space (direct writing to XML)
	 * @param[in]  orientation       orientation in PARENT space (direct writing to XML)
	 * @param[in]  operationsToDo    binary mask of operation to write to do, see @ref MGE::AxisGizmo::Modes
	 * @param[in]  operationsToSave  binary mask of operation to write to XML (via @ref updateXML), see @ref MGE::AxisGizmo::Modes
	 */
	void updateNodes(
		const Ogre::Vector3& position,
		const Ogre::Vector3& scale,
		const Ogre::Quaternion& orientation,
		int operationsToDo,
		int operationsToSave
	);
	
	/**
	 * @brief update XML for node
	 * 
	 * @param[in]  node             scene node to get and modify XML node
	 * @param[in]  position         position in PARENT space (direct writing to XML)
	 * @param[in]  orientation      orientation in PARENT space (direct writing to XML)
	 * @param[in]  scale            scale in LOCAL space (direct writing to XML)
	 * @param[in]  operations       binary mask of operation to write to XML, see @ref MGE::AxisGizmo::Modes
	 */
	void updateXML(
		Ogre::SceneNode* node,
		const Ogre::Vector3& position,
		const Ogre::Vector3& scale,
		const Ogre::Quaternion& orientation,
		int operations = MGE::AxisGizmo::MOVE | MGE::AxisGizmo::ROTATE | MGE::AxisGizmo::SCALE
	);
	
	
	/**
	 * @brief handle selection change in comoboxes
	 */
	bool handleCombobox(const CEGUI::EventArgs& args);
	
	/**
	 * @brief handle key input ion Combobox
	 */
	bool handleComboboxKey(const CEGUI::EventArgs& args);
	
	/**
	 * @brief handle selection change in checkboxes
	 */
	bool handleCheckbox(const CEGUI::EventArgs& args);
	
	/**
	 * @brief handle preview and apply button in @ref winTransform
	 */
	bool valEditCallback(const CEGUI::EventArgs& args);
	
	/**
	 * @brief handle new node buttons in @ref winNodeInfo
	 */
	bool newNodeCallback(const CEGUI::EventArgs& args);
	
	/**
	 * @brief handle apply button in @ref winNodeInfo
	 */
	bool nodeApplyCallback(const CEGUI::EventArgs& args);
	
	/**
	 * @brief handle "select parent" button
	 */
	bool selectParentCallback(const CEGUI::EventArgs& args);
	
	/**
	 * @brief set rotation mode to @a id
	 * 
	 * @note update @ref rotationMode and switch @ref winTransform between quaterion mode and euler angles mode
	 */
	void setRotationMode(int id);
	
	/**
	 * @brief set pivot mode to @a id
	 * 
	 * @note update @ref selTransformPivot (when call with id != NONE) and @ref pivotNode (always)
	 */
	void setTransformPivot(int id = NONE);
	
	/**
	 * @brief create and add item to CEGUI Combobox
	 * 
	 * @param win     pointer to combobox
	 * @param txt     item text
	 * @param id      item numeric ID
	 * @param defId   default item ID in this combobox (when @a id == @a defId set current text in combobox to @a txt)
	 * @param colour  colour of text in item
	 */
	void addItemToCombobox(CEGUI::Combobox* win, const CEGUI::String& txt, int id, int defId);
	
	Ogre::Node::TransformSpace  selTransformSpace;  ///< X,Y,Z axis in gizmo and in numeric field is axis of WORLD, PARENT or LOCAL space
	int                         selValueSpace;      ///< @brief numeric values are relative to WORLD, PARENT or LOCAL space;
	                                                ///< @note  LOCAL means "updating mode" (showing values are zero / identity,
	                                                ///< @note  entered values are adding to current respecting @ref selTransformSpace)
	int                         selTransformPivot;  ///< rotation pivot - rotation around seleced point
	int                         selGizmoOperation;  ///< current / selected gizmo operation (moving, scaling, rotating)
	int                         rotationMode;       ///< current / selected gizmo rotation mode
	
	Ogre::SceneNode*            targetNode;         ///< pointer to target of operation scene node
	Ogre::SceneNode*            pivotNode;          ///< pointer to pivot (gizmo attached) scene node
	Ogre::SceneNode*            marker3DNode;       ///< pointer to "3D cursor" (external pivot marker) scene node
	Ogre::SceneNode*            groupNode;          ///< pointer to group "center" scene node
	Ogre::Vector3               initScale;          ///< initial targetNode scale
	Ogre::Quaternion            initOrientation;    ///< initial targetNode orientation
	Ogre::Vector3               initPosition;       ///< initial targetNode position
	bool                        gizmoInMoveMode;    ///< gizmo moving in progress (@ref zeroOffset is valid)
	Ogre::Vector3               gizmoZeroOffset;    ///< offset between clickpoint and node position (used for moving)
	
	std::map<Ogre::SceneNode*, Ogre::Quaternion>  initOrientations;  ///< initial orientations for selected group members
	std::map<Ogre::SceneNode*, Ogre::Vector3>     initPositions;     ///< initial positions for selected group members
	
	MGE::LoadingContext         context;
	
	float                       rotateSpeedFactor;
	float                       negScaleFactor;
	float                       posScaleFactor;
	
	CEGUI::Window*              winEditor;
	CEGUI::Combobox*            winTransformSpace;
	CEGUI::Combobox*            winValueSpace;
	CEGUI::Combobox*            winTransformPivot;
	CEGUI::Combobox*            winOperation;
	CEGUI::ToggleButton*        winShowTriggers;
	CEGUI::ToggleButton*        winIdividualObjects;
	
	CEGUI::Window*              winTransform;
	CEGUI::Spinner*             winPositionX;
	CEGUI::Spinner*             winPositionY;
	CEGUI::Spinner*             winPositionZ;
	CEGUI::Spinner*             winScaleX;
	CEGUI::Spinner*             winScaleY;
	CEGUI::Spinner*             winScaleZ;
	CEGUI::Spinner*             winRotationX;
	CEGUI::Spinner*             winRotationY;
	CEGUI::Spinner*             winRotationZ;
	CEGUI::Spinner*             winRotationW;
	CEGUI::Combobox*            winRotationMode;
	
	CEGUI::Window*              winNodeInfo;
	CEGUI::Editbox*             winNodeName;
	CEGUI::Editbox*             winItemName;
	CEGUI::Combobox*            winMesh;
	
	pugi::xml_document*         dotSceneFile;
	
	enum ComboboxItem {
		NONE   = 0,
		
		TRANSFORM_POINT_WORLD,
		TRANSFORM_POINT_PARENT,
		TRANSFORM_POINT_OBJECT,
		TRANSFORM_POINT_MARKER,
		
		TRANSFORM_VALUES_GLOBAL,
		TRANSFORM_VALUES_LOCAL,
		TRANSFORM_VALUES_OFFSET,
		
		ROT_QUATERNION,
		ROT_XYZ,
		ROT_XZY,
		ROT_YXZ,
		ROT_YZX,
		ROT_ZXY,
		ROT_ZYX,
	};
	
	struct SelectionSet :  public MGE::SelectionSetTemplate<Ogre::SceneNode*, 0xFFFFFFFF, SelectionSet> {
		SelectionSet() { }
		
		using MGE::SelectionSetTemplate<Ogre::SceneNode*, 0xFFFFFFFF, SelectionSet>::select;
		
		/// @copydoc MGE::Selection::SelectionSetBase::select
		bool select( MGE::RayCast::ResultsPtr searchResults, int _selectSwitchMode, int _selectionMode ) override;
		
		/// @copydoc MGE::SelectionSetTemplate::canSelect
		static bool canSelect(Ogre::SceneNode* obj, int mode) {
			return !MGE::Editor::getPtr()->axisGizmo->isGizmoNode(obj);
		}
		
		/// @copydoc MGE::SelectionSetTemplate::markSelection
		static void markSelection(Ogre::SceneNode* obj, bool selection, int mode);
		
		/// @copydoc MGE::SelectionSetTemplate::onSelectionChanged
		static void onSelectionChanged();
	};
	
	/// selection set for selected scene node
	SelectionSet                  selectionSet;
	
	/// pointer to axis gizmo widget
	MGE::AxisGizmo*               axisGizmo;
	
	/// settings set for selection marker
	MGE::VisualMarkerSettingsSet  markerSettings;
	
	class ContextMenu;
	ContextMenu*                  contextMenu;
};

/// @}

}
