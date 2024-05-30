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

#include "modules/utils/Micropather.h"

namespace MGE { struct BasePrototype; }
namespace MGE { struct PrototypeFactory; }

#include <list>
#include <unordered_map>

namespace CEGUI { class ItemListbox; }

namespace MGE {

/// @addtogroup Modules
/// @{
/// @file

/**
 * @brief Window with information about current selected actor
 */
class WorldMap :
	public MGE::GenericWindows::BaseWindowOwner,
	public MGE::Module,
	public MGE::MainLoopListener,
	public MGE::SaveableToXML<WorldMap>,
	public MGE::Singleton<WorldMap>
{
public:
	/// @copydoc MGE::GenericWindows::BaseWindowOwner::show
	void show(const CEGUI::String& name = CEGUI::String::GetEmpty()) override;
	
	/// @copydoc MGE::MainLoopListener::update
	/// (used to moving units from base to mission area)
	bool update(float gameTimeStep, float realTimeStep) override;
	
	/// type of static function for register as @ref unitOnTheActionSite
	///
	/// @param proto    pointer to prototype of the unit
	/// @param personel reference to map of prototype pointers to personel/equipment and its quantity
	typedef void (*UnitOnTheActionSiteListener)(const MGE::BasePrototype* proto, const std::unordered_map<MGE::BasePrototype*, int>& personel);
	
	/// static function used for proccessing xml tag in map config and .scene file
	UnitOnTheActionSiteListener unitOnTheActionSite;
	
	/// Name of XML tag for @ref MGE::SaveableToXML::getXMLTagName.
	inline static const char* xmlStoreRestoreTagName = "WorldMap";
	
	/// @copydoc MGE::SaveableToXMLInterface::restoreFromXML
	virtual bool restoreFromXML(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context) override;
	
	/// @copydoc MGE::SaveableToXMLInterface::storeToXML
	virtual bool storeToXML(pugi::xml_node& xmlNode, bool onlyRef) const override;
	
	/**
	 * @brief constructor
	 * 
	 * @param[in] baseWin      pointer to parent (tabs, frame, etc) window object
	 * @param[in] configFile   world map config file name
	 * @param[in] configGroup  resource group for world map config
	 * @param[in] missionPos   mission position in world map coordinates
	 */
	WorldMap( 
		MGE::GenericWindows::BaseWindow* baseWin,
		const Ogre::String& configFile,
		const Ogre::String& configGroup,
		const Ogre::Vector2& missionPos
	);
	
	/**
	 * @brief create WorldMap based on XML configuration
	 * 
	 * @param[in] xmlNode           XML configuration node
	 */
	static WorldMap* create(const pugi::xml_node& xmlNode);
	
	/// destructor
	virtual ~WorldMap();
	
protected:
	struct SceneGraph;
	struct BaseOnWorldMap;
	struct UnitInBase;
	struct UnitInBaseItem;
	struct UnitOnWorldMap;
	
	/**
	 * @brief handle hide equipment base window
	 * 
	 * @param[in] args - CEGUI Event detail/description
	 */
	bool handleHideBaseWin(const CEGUI::EventArgs& args);
	
	/**
	 * @brief handle open base subwindow
	 * 
	 * @param[in] args - CEGUI Event detail/description
	 */
	bool handleOpenBase(const CEGUI::EventArgs& args);
	
	/**
	 * @brief handle click on unit - show unit info
	 * 
	 * @param[in] args - CEGUI Event detail/description
	 */
	bool handleUnitClick(const CEGUI::EventArgs& args);
	
	/**
	 * @brief handle click on personel unit - show personel info
	 * 
	 * @param[in] args - CEGUI Event detail/description
	 */
	bool handlePersonelClick(const CEGUI::EventArgs& args);
	
	/**
	 * @brief handle changing quantity of selected personel
	 * 
	 * @param[in] args - CEGUI Event detail/description
	 */
	bool handlePersonelNumChanged(const CEGUI::EventArgs& args);
	
	/**
	 * @brief handle changing quantity of units
	 * 
	 * @param[in] args - CEGUI Event detail/description
	 */
	bool handleSendNumChanged(const CEGUI::EventArgs& args);
	
	/**
	 * @brief handle sending unit from base to map
	 * 
	 * @param[in] args - CEGUI Event detail/description
	 */
	bool handleSend(const CEGUI::EventArgs& args);
	
	/// pointer to world map (graphics) window
	CEGUI::Window*         mapWin;
	
	/// world map width
	float                  mapWidth;
	
	/// world map height
	float                  mapHeight;
	
	/// pointer to equipment base window
	CEGUI::Window*         baseWin;
	
	/// pointer to unit description window
	CEGUI::Window*         unitDesc;
	
	/// pointer to unit send button
	CEGUI::Window*         unitSend;
	
	/// pointer to unit quantity spinner
	CEGUI::Spinner*        unitSendNum;
	
	/// pointer to units list in equipment base window object
	CEGUI::ListWidget*     unitsList;
	
	/// pointer to personel list in equipment base window object
	CEGUI::ScrollablePane* personelList;
	
	/// set of @ref personelList "item" sub-windows
	std::unordered_set<CEGUI::Window*> personelListItems;
	
	/// clear personelList (destroy all children from @ref personelListItems and clear this set)
	void clearPersonelList();
	
	/// default resource group for search units images (used when not set in actor properties)
	std::string                                   defaultImagesGroup;
	
	/// list of all bases
	std::list<MGE::WorldMap::BaseOnWorldMap*>     bases;
	
	/// pointer to currently selected base
	MGE::WorldMap::BaseOnWorldMap*                currentSelectedBase;
	
	/// pointer to current selected item on unitsList
	MGE::WorldMap::UnitInBase*                    currentSelectedUnit;
	
	/// map of currently selected personel
	std::unordered_map<MGE::BasePrototype*, int>  currentSelectedPersonels;
	
	/// list of vehicles currently on the way to mission point
	std::list<MGE::WorldMap::UnitOnWorldMap*>     unitsOnTheWay;
	
	/// find base by position on world map
	MGE::WorldMap::BaseOnWorldMap*                findBase(int x, int y);
	
	/// find base by cegui window (window with base mark or it children)
	MGE::WorldMap::BaseOnWorldMap*                findBase(CEGUI::Window* win);
};

/// @brief Road graph of world map for path finding.
///
/// MGE::MicroPather::Graph implementation for searching path on png file.
///
/// See too @ref PathFinding.
struct MGE::WorldMap::SceneGraph MGE_CLASS_FINAL : public MGE::MicroPather::Graph {
	/// constructror
	SceneGraph(const std::string& mapFile);
	
	/// destructor
	~SceneGraph() {}
	
	/**
	 * @brief calculate cost (length) betwen stateStart and stateEnd
	 * 
	 * @param[in] stateStart - state of start point
	 * @param[in] stateEnd   - state of finish point
	 */
	float LeastCostEstimate(
		MICROPATHER_STATE_TYPE stateStart,
		MICROPATHER_STATE_TYPE stateEnd
	) override;
	
	/**
	 * @brief add single StateCost
	 * 
	 * @param[in]  src    - base point
	 * @param[in]  a      - offset from base point
	 * @param[in]  b      - offset from base point
	 * @param[in]  cost   - cost (length) betwen base and offset point
	 * @param[out] states - vector tu push new StateCost
	 */
	void makeNeighbor(
		MICROPATHER_STATE_TYPE src, int a, int b, float cost,
		std::vector< MGE::MicroPather::StateCost >* states
	);
	
	/**
	 * @brief add all neighbor of current state
	 * 
	 * @param[in]  state  - current state
	 * @param[out] states - vector tu push neighbor
	 */
	void  AdjacentCost(
		MICROPATHER_STATE_TYPE state,
		std::vector< MGE::MicroPather::StateCost >* states
	) override;
	
	/**
	 * @brief print state coordinates (for debug only)
	 * 
	 * @param[in]  state
	 */
	void  PrintStateInfo(MICROPATHER_STATE_TYPE state) override;
	
	/// set of road points,
	/// alternative we can use 2D table of bool values (road/not road) - faster but more memory in typical cases
	std::set<MICROPATHER_STATE_TYPE> roads;
	
private:
	enum read_png_error {
		E_FOPEN       = -1,
		E_READ_STRUCT = -2,
		E_INFO_STRUCT = -3,
		E_PNG_TYPE    = -4,
	};
	
	int read_png(const char* file_name);
};

/// struct describe base on world map
struct MGE::WorldMap::BaseOnWorldMap {
	/// constructor - based on configuration xml element
	BaseOnWorldMap(const pugi::xml_node& xmlNode);
	
	/// @copydoc MGE::SaveableToXMLInterface::restoreFromXML
	bool restoreFromXML(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context);
	
	/// x world map coordinate of location
	int   x;
	
	/// y world map coordinate of location
	int   y;
	
	/// pointer to "button" window on world map
	CEGUI::Window* win;
	
	/// list of base equipment
	std::list<MGE::WorldMap::UnitInBase> units;
	
	/// path from base to mission point
	std::vector<MICROPATHER_STATE_TYPE> path;
	
	/// @copydoc MGE::SaveableToXMLInterface::storeToXML
	bool storeToXML(pugi::xml_node& xmlNode, bool onlyRef) const;
};

/// struct describe unit in base on world map
struct MGE::WorldMap::UnitInBase {
	/// prototype for creating actor from base-unit
	MGE::BasePrototype* proto;
	
	/// quantity of units with this prototype in base
	int quantity;
	
	/// constructor - based on xml
	UnitInBase(const pugi::xml_node& xmlNode);
	
	/// @copydoc MGE::SaveableToXMLInterface::storeToXML
	bool storeToXML(pugi::xml_node& xmlNode, bool onlyRef) const;
};

/// struct describe vehicle on the way
struct MGE::WorldMap::UnitOnWorldMap MGE_CLASS_FINAL {
	/// constructor - create vehicle on the way from prototypes and quantity info
	UnitOnWorldMap(
		const MGE::BasePrototype* unit,
		const std::unordered_map<MGE::BasePrototype*, int>& personelList,
		int quantity,
		MGE::WorldMap::BaseOnWorldMap* fromBase
	);
	
	/// constructor - create vehicle on the way from xml
	UnitOnWorldMap(const pugi::xml_node& xmlNode);
	
	/// destructor - delete world-map vehicle marker window
	~UnitOnWorldMap();
	
	/// @copydoc MGE::SaveableToXMLInterface::storeToXML
	bool storeToXML(pugi::xml_node& xmlNode, bool onlyRef) const;
	
	/// update position of world-map vehicle marker window
	void update(float gameTimeStep);
	
	/// prototype of vehicle
	const MGE::BasePrototype* proto;
	
	/// maps of personel prototypes and quantity
	std::unordered_map<MGE::BasePrototype*, int> personel;
	
	/// curent position in base-destination path
	float position;
	
	/// quantity of vehicle units in this same place (on this same marker)
	int quantity;
	
	/// unit speed
	float speed;
	
	/// parent base of this unit (for getting base-destination path)
	MGE::WorldMap::BaseOnWorldMap* fromBase;
	
private:
	struct PersonelSerializer;
	CEGUI::Window* win;
	CEGUI::UDim x_offset, y_offset;
	void initUnitOnWorldMap();
};

/// struct for items in actionQueueList
struct MGE::WorldMap::UnitInBaseItem : public CEGUI::StandardItem {
	MGE::WorldMap::UnitInBase* unit;
	
	UnitInBaseItem(MGE::WorldMap::UnitInBase* u);
	
	virtual bool operator==(const CEGUI::GenericItem& other) const override;
};

/// @}

}
