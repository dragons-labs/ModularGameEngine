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

#include "modules/gui/WorldMap.h"

#include "gui/InputAggregator4CEGUI.h"
#include "gui/utils/CeguiString.h"
#include "gui/utils/CeguiStretchedImage.h"
#include "data/utils/OgreResources.h"

#include "data/structs/factories/PrototypeFactory.h"

#include <CEGUI/RendererModules/Ogre/Renderer.h>

#include <iostream>

/*  ----====  constructor, destructor and load()  ====----  */

/**
@page XMLSyntax_WorldMapConfig  WorldMap Config File

@section XMLSyntax_WorldMapConfigSyntax WorldMap config file syntax

WorldMap config file is XML file with \<worldMap\> root node used to describe world map.

@subsection XMLSyntax_WorldMapConfig_RootNode \<worldMap\>

@c \<worldMap\> have next attributes:
	- @c width   with width of world map (used for word map 2D coordinate system)
	- @c height  with height of world map (used for word map 2D coordinate system)
	- @c roadsFileName with road layout file name (this file is used for tracepath to search route from bases to action point)
	- @c roadsGroup with resource group for search road layout file
	- @c defaultImagesGroup with default resource group for search units images (used when not set in actor properties)
	- @c priority   (optional) priority used to select between files with this same name and this same resource group (default 0, used is file with highest value)
	.
	and next subnodes:
	- @c \<mapImage\> with texture file representing world map outlook, specified by attributes:
		- @c file file name in Ogre resources system
		- @c group group name for search this file
	- @c \<base\> describing single base on map, can be used multiple time

@subsection XMLSyntax_WorldMapConfig_BaseNode \<base\>

@c \<base\> describing base on world map and have next attributes:
	- @c x x (horizontal) coordinate of base position in word map coordinate system
	- @c y y (vertical) coordinate of base position in word map coordinate system
	- @c infoA first  line of map info text on world map
	- @c infoB second line of map info text on world map
	- @c infoC third  line of map info text on world map
	.
	and next subnodes:
	- @c \<unit\> describing single unit (car, boat, plane, etc) in base, can be used multiple time

@subsection XMLSyntax_WorldMapConfig_UnitNode \<unit\>

@c \<unit\> describing unit in base (via actor prototype) and have next attributes:
	- @c name, @c file, @c group like in @ref XMLNode_PrototypeRef nodes (see @ref XMLSyntax_PrototypeConfigFile)
	- @c quantity quantity of this unit in this base
*/

MGE::WorldMap::WorldMap(
	MGE::GenericWindows::BaseWindow* _baseWin,
	const Ogre::String& configFile,
	const Ogre::String& configGroup,
	const Ogre::Vector2& missionPos
) : 
	MGE::GenericWindows::BaseWindowOwner(_baseWin),
	MGE::SaveableToXML<WorldMap>(501, 201)
{
	LOG_INFO("Initialise WorldMap, missionPos=" << missionPos);
	
	baseWin = CEGUI::WindowManager::getSingleton().loadLayoutFromFile("WorldMap/UnitsBase.layout");
	
	unitsList = static_cast<CEGUI::ListWidget*>(baseWin->getChild( "AvailableUnits" ));
	unitsList->subscribeEvent(
		CEGUI::ListWidget::EventSelectionChanged,
		CEGUI::Event::Subscriber(&MGE::WorldMap::handleUnitClick, this)
	);
	
	personelList = static_cast<CEGUI::ScrollablePane*>(baseWin->getChild( "AvailablePersonel" ));
	
	unitDesc = baseWin->getChild( "Description" );
	unitSend = baseWin->getChild( "Send" );
	MGE::GUISystem::getPtr()->setTranslatedText( unitSend );
	unitSend->subscribeEvent(
		CEGUI::PushButton::EventClicked,
		CEGUI::Event::Subscriber(&MGE::WorldMap::handleSend, this)
	);
	unitSendNum = static_cast<CEGUI::Spinner*>(baseWin->getChild( "SendNum" ));
	unitSendNum->subscribeEvent(
		CEGUI::Spinner::EventValueChanged,
		CEGUI::Event::Subscriber(&MGE::WorldMap::handleSendNumChanged, this)
	);
	
	static_cast<CEGUI::FrameWindow*>(baseWin)->getCloseButton()->subscribeEvent(
		CEGUI::PushButton::EventClicked,
		CEGUI::Event::Subscriber(&MGE::WorldMap::handleHideBaseWin, this)
	);
	baseWin->hide();
	
	// this is doing after constructor due to usage of getPtr() ...
	LOG_INFO("Configure WorldMap based on: " + configFile + " from: " + configGroup + " resource group");
	
	SceneGraph* sceneGraph = NULL;
	MGE::MicroPather* pather = NULL;
	
	// open config xml file
	pugi::xml_document xmlFile;
	auto xmlRootNode = MGE::XMLUtils::openXMLFile(xmlFile, MGE::OgreResources::getResourcePath(configFile, configGroup, "worldMap"sv).c_str(), "worldMap");
	
	// read road map size
	mapWidth   = xmlRootNode.attribute("width").as_float(0);
	mapHeight  = xmlRootNode.attribute("height").as_float(0);
	
	// read default resource group for search units images
	defaultImagesGroup = xmlRootNode.attribute("defaultImagesGroup").as_string("UnitsImages");
	
	// read and parse road map file
	sceneGraph = new SceneGraph( MGE::OgreResources::getResourcePath(
		xmlRootNode.attribute("roadsFileName").as_string(),
		xmlRootNode.attribute("roadsGroup").as_string()
	) );
	pather     = new MGE::MicroPather( sceneGraph );
	
	// map (texture) image
	{
		auto xmlNode = xmlRootNode.child("mapImage");
		CEGUI::ImageManager::getSingleton().addBitmapImageFromFile(
			"worldMap",
			xmlNode.attribute("file").as_string(),
			xmlNode.attribute("group").as_string("WorldMaps")
		);
		mapWin = getWindow()->getChild("WorldMap");
		mapWin->setProperty("Image", "worldMap");
	}
	
	// mission location
	CEGUI::Window* missionPointWin = CEGUI::WindowManager::getSingleton().loadLayoutFromFile("WorldMap/Marker-Point.layout");
	CEGUI::Window* point = missionPointWin->getChild( "Point" );
	MGE::GUISystem::getPtr()->setTranslatedText( missionPointWin->getChild( "Text" ), "Mission" );
	missionPointWin->setPosition(CEGUI::UVector2(
		CEGUI::UDim( missionPos.x/mapWidth,  0) - (point->getXPosition() + point->getWidth() * 0.5),
		CEGUI::UDim( missionPos.y/mapHeight, 0) - (point->getYPosition() + point->getHeight() * 0.5)
	));
	missionPointWin->show();
	mapWin->addChild(missionPointWin);
	
	// base sub window is child of world map
	mapWin->addChild(baseWin);
	
	// base loacation and info
	for (auto xmlNode : xmlRootNode.children("base")) {
		MGE::WorldMap::BaseOnWorldMap* base = new MGE::WorldMap::BaseOnWorldMap(xmlNode);
		bases.push_back(base);
		
		base->win = CEGUI::WindowManager::getSingleton().loadLayoutFromFile("WorldMap/Marker-Base.layout");
		point = base->win->getChild( "Point" );
		base->win->setPosition(CEGUI::UVector2(
			CEGUI::UDim( static_cast<float>(base->x)/mapWidth,  0) - (point->getXPosition() + point->getWidth() * 0.5),
			CEGUI::UDim( static_cast<float>(base->y)/mapHeight, 0) - (point->getYPosition() + point->getHeight() * 0.5)
		));
		
		base->win->getChild( "TextLine1" )->setText( xmlNode.attribute("infoA").as_string() );
		base->win->getChild( "TextLine2" )->setText( xmlNode.attribute("infoB").as_string() );
		base->win->getChild( "TextLine3" )->setText( xmlNode.attribute("infoC").as_string() );
		
		base->win->setUserData(base);
		base->win->subscribeEvent(
			CEGUI::Window::EventClick,
			CEGUI::Event::Subscriber(&MGE::WorldMap::handleOpenBase, this)
		);
		
		base->win->show();
		mapWin->addChild(base->win);
		
		// calculate (and seve to "base") path from base to missionPos
		if (pather) {
			float totalCost;
			pather->Reset();
			int result = pather->Solve(
				MICROPATHER_STATE_TYPE(missionPos.x, missionPos.y),
				MICROPATHER_STATE_TYPE(base->x, base->y),
				&(base->path),
				&totalCost
			);
			if ( result != MGE::MicroPather::SOLVED ) {
				LOG_WARNING("Unable to find path from base \"" << xmlNode.attribute("infoA").as_string() << "\" to mission point" );
			} else {
				LOG_INFO("Find path from base \"" << xmlNode.attribute("infoA").as_string() << "\" to mission point" );
			}
		}
	}
	
	delete pather;
	delete sceneGraph;
	
	unitOnTheActionSite = NULL;
	
	MGE::Engine::getPtr()->mainLoopListeners.addListener(this, POST_RENDER_GUI);
}

MGE::WorldMap::~WorldMap(void) {
	LOG_INFO("destroy WorldMap");
	
	for (auto& iter : bases)
		delete iter;
	bases.clear();
	
	for (auto& iter : unitsOnTheWay)
		delete iter;
	unitsOnTheWay.clear();
	
	MGE::Engine::getPtr()->mainLoopListeners.remListener(this);
	
	CEGUI::WindowManager::getSingleton().destroyWindow(baseWin);
	CEGUI::ImageManager::getSingleton().destroy("worldMap");
	MGE::GUISystem::getPtr()->getRenderer()->destroyTexture("worldMap");
	
	// window->remClient() is in (automatic called) BaseWindowOwner destructor ... and can destroy baseWin too
}

/**
@page XMLSyntax_MapAndSceneConfig

@subsection XMLNode_WorldMap \<WorldMap\>

@c \<WorldMap\> is used for enabled and configure GUI (sub)widow with mini map of action scene. It have required subnodes:
	- @ref XMLNode_BaseWin
	- @c \<File\> with @ref XMLSyntax_WorldMapConfig specified by attributes:
		- @c name file name in Ogre resource system
		- @c group group name for search this file
	- @c \<ActionPosition\> XML_Vector2 with position on world map to put mission marker
*/

MGE::WorldMap* MGE::WorldMap::create(const pugi::xml_node& xmlNode) {
	LOG_INFO("Load / create WorldMap based on config xml node");
	
	MGE::GenericWindows::BaseWindow* baseWin = MGE::GenericWindows::Factory::getPtr()->get(xmlNode);
	if(!baseWin) {
		throw std::logic_error("Could not create base window for WorldMap");
	}
	
	auto xmlSubNode = xmlNode.child("File");
	if (xmlSubNode) {
		return new MGE::WorldMap(
			baseWin, 
			xmlSubNode.attribute("name").as_string(), 
			xmlSubNode.attribute("group").as_string("Map_Scene"), 
			MGE::XMLUtils::getValue(xmlNode.child("ActionPosition"), Ogre::Vector2::ZERO)
		);
	} else {
		throw std::logic_error("No config file for WorldMap");
	}
}

MGE_CONFIG_PARSER_MODULE_FOR_XMLTAG(WorldMap) {
	return MGE::WorldMap::create(xmlNode);
}


/*  ----====  store, restore interface  ====----  */

bool MGE::WorldMap::storeToXML(pugi::xml_node& xmlNode, bool /*onlyRef*/) const {
	LOG_INFO("store WorldMap data");
	
	auto xmlSubNode = xmlNode.append_child("bases");
	for (const auto& iter : bases) {
		auto xmlSubSubNode = xmlSubNode.append_child("base");
		iter->storeToXML( xmlSubSubNode, false );
	}
	
	xmlSubNode = xmlNode.append_child("unitsOnTheWay");
	for (const auto& iter : unitsOnTheWay) {
		auto xmlSubSubNode = xmlSubNode.append_child("vehicle");
		iter->storeToXML( xmlSubSubNode, false );
	}
	
	return true;
}

bool MGE::WorldMap::restoreFromXML(const pugi::xml_node& xmlNode, const MGE::LoadingContext* /*context*/) {
	LOG_INFO("restore WorldMap data");
	
	for (auto xmlSubNode : xmlNode.child("bases").children("base")) {
		// read base identification - (x, y) position on world map
		int x = xmlSubNode.attribute("x").as_int();
		int y = xmlSubNode.attribute("y").as_int();
		
		// find base with this position
		BaseOnWorldMap* base = MGE::WorldMap::getPtr()->findBase(x, y);
		if (!base)
			throw std::logic_error("Can't find base at x=" + std::to_string(x) + " y=" + std::to_string(y));
		base->restoreFromXML(xmlNode, nullptr);
	}
	
	for (auto xmlSubNode : xmlNode.child("unitsOnTheWay").children("vehicle")) {
		unitsOnTheWay.push_back( new MGE::WorldMap::UnitOnWorldMap(xmlSubNode) );
	}
	
	return true;
}


/*  ----====  main loop update  ====----  */

bool MGE::WorldMap::update(float gameTimeStep, float realTimeStep) {
	if (gameTimeStep == 0.0f) // game is paused
		return false;
	
	bool canput = true;
	auto iter = unitsOnTheWay.begin(); // don't use `for(auto& it : set)` because of using `set.erase(it)` in the loop
	while( iter != unitsOnTheWay.end() ) {
		if((*iter)->position > 0) {
			(*iter)->update(gameTimeStep);
			++iter;
		} else if (canput) {
			LOG_DEBUG("vehicleOnTheActionSite");
			
			if (unitOnTheActionSite)
				unitOnTheActionSite((*iter)->proto, (*iter)->personel);
			
			// remove unitsOnTheWay
			if (--((*iter)->quantity) <= 0) {
				delete* iter;
				unitsOnTheWay.erase(iter++);
			} else {
				++iter; // we put only one vehicle in one render cycle - otherwise we have problem with findFreePosition()
				canput = false;
			}
		}
	}
	
	return true;
}


/*  ----====  GUI handlers  ====----  */

void MGE::WorldMap::show(const CEGUI::String& name) {
	if (name.empty())
		window->show("WorldMap");
	else
		window->show(name);
}

MGE::WorldMap::UnitInBaseItem::UnitInBaseItem(MGE::WorldMap::UnitInBase* u) :
	CEGUI::StandardItem(
		STRING_TO_CEGUI( u->proto->getPropertyValue<std::string>("_name", MGE::EMPTY_STRING) ),
		STRING_TO_CEGUI( u->proto->getPropertyValue<std::string>("_img", MGE::EMPTY_STRING) ),
		0
	),
	unit(u)
{
	if ( ! d_icon.empty() ) {
		if ( ! CEGUI::ImageManager::getSingleton().isDefined( d_icon ) ) {
			CEGUI::ImageManager::getSingleton().addBitmapImageFromFile(
				d_icon, d_icon,
				STRING_TO_CEGUI( u->proto->getPropertyValue<std::string>("_imgGrp", MGE::EMPTY_STRING) )
			);
		}
		d_text = "[image-height='64'][image-width='100'][aspect-lock='true'][image='" + d_icon + "']\n[colour='FF000000']" + d_text;
		d_icon = "";
	}
}

bool MGE::WorldMap::UnitInBaseItem::operator==(const CEGUI::GenericItem& other) const {
	const UnitInBaseItem* myOther = dynamic_cast<const UnitInBaseItem*>(&other);
	if (myOther && unit != myOther->unit)
		return false;
	return CEGUI::GenericItem::operator==(other);
}

void MGE::WorldMap::clearPersonelList() {
	for (auto& iter : personelListItems)
		personelList->destroyChild( iter );
	personelListItems.clear();
}

bool MGE::WorldMap::handleOpenBase(const CEGUI::EventArgs& args) {
	CEGUI::Window* cwin = static_cast<const CEGUI::WindowEventArgs&>(args).window;
	
	// currentSelectedBase = static_cast<MGE::WorldMap::BaseOnWorldMap *>(cwin->getUserData());
	currentSelectedBase = findBase(cwin);
	
	if (!currentSelectedBase) {
		LOG_DEBUG("handleOpenBase can't find base for window: " << cwin->getName());
		
		LOG_DEBUG("  " << cwin->isCursorPassThroughEnabled());
		return false;
	}
	
	unitsList->clearList();
	clearPersonelList();
	
	currentSelectedUnit = NULL;
	unitSend->setProperty("Disabled", "True");
	unitDesc->setText("");
	
	for (auto& iter : currentSelectedBase->units) {
		unitsList->addItem( new UnitInBaseItem(&iter) );
	}
	
	baseWin->show();
	baseWin->activate();
	
	return true;
}

bool MGE::WorldMap::handleUnitClick(const CEGUI::EventArgs& args) {
	LOG_DEBUG("handleUnitClick");
	
	UnitInBaseItem* selectedItem = static_cast<UnitInBaseItem*>( unitsList->getFirstSelectedItem() );
	if (selectedItem) {
		// set description
		unitDesc->setText(
			STRING_TO_CEGUI( selectedItem->unit->proto->getPropertyValue<std::string>("_desc", MGE::EMPTY_STRING) )
		);
		
		if (currentSelectedUnit != selectedItem->unit) {
			currentSelectedUnit = selectedItem->unit;
			
			unitSendNum->setProperty("MaximumValue", CEGUI::PropertyHelper<int>::toString(currentSelectedUnit->quantity));
			if (currentSelectedUnit->quantity > 0) {
				unitSendNum->setCurrentValue(1);
				unitSend->setProperty("Disabled", "False");
			} else {
				unitSendNum->setCurrentValue(0);
				unitSend->setProperty("Disabled", "True");
			}
			
			clearPersonelList();
			
			auto propList = currentSelectedUnit->proto->getPropertyValue< std::list<std::string> >("PosiblePersonel", {});
			for (auto& iter : propList) {
				LOG_DEBUG("  - " << iter);
				
				pugi::xml_document xmlDoc;
				xmlDoc.load_string( ("<p "sv + iter + " />"sv).c_str() );
				MGE::BasePrototype* proto = MGE::PrototypeFactory::getPtr()->getPrototype( xmlDoc.child("p") );
				
				if (proto) {
					CEGUI::Window* personelItem = CEGUI::WindowManager::getSingleton().loadLayoutFromFile("WorldMap/PersonelListItem.layout");
					
					personelListItems.insert(personelItem);
					
					personelItem->getChild("Text")->setText(
						STRING_TO_CEGUI( proto->getPropertyValue<std::string>("_code", MGE::EMPTY_STRING) )
					);
					personelItem->setTooltipText(
						STRING_TO_CEGUI(  proto->getPropertyValue<std::string>("_name", MGE::EMPTY_STRING) )
					);
					personelItem->setUserData(const_cast<void*>(static_cast<const void*>(proto)));
					
					MGE::setStretchedImage(
						personelItem->getChild("Image"),
						proto->getPropertyValue<std::string>("_img", MGE::EMPTY_STRING),
						proto->getPropertyValue<std::string>("_imgGrp", defaultImagesGroup)
					);
					
					CEGUI::Spinner* spinner = static_cast<CEGUI::Spinner*>(personelItem->getChild("Count"));
					spinner->subscribeEvent(
						CEGUI::Spinner::EventValueChanged,
						CEGUI::Event::Subscriber(&MGE::WorldMap::handlePersonelNumChanged, this)
					);
					spinner->setUserData(const_cast<void*>(static_cast<const void*>(proto)));
					spinner->setCurrentValue(0);
					
					personelItem->subscribeEvent(
						CEGUI::Window::EventClick,
						CEGUI::Event::Subscriber(&MGE::WorldMap::handlePersonelClick, this)
					);
					
					personelItem->show();
					personelList->addChild(personelItem);
				} else {
					LOG_WARNING("can't find prototype");
				}
			}
		}
	}
	
	return true;
}

bool MGE::WorldMap::handlePersonelClick(const CEGUI::EventArgs& args) {
	LOG_DEBUG("handlePersonelClick");
	
	MGE::BasePrototype* proto = static_cast<MGE::BasePrototype*>(
		static_cast<const CEGUI::WindowEventArgs&>(args).window->getUserData()
	);
	if (proto) {
		unitDesc->setText( 
			STRING_TO_CEGUI( proto->getPropertyValue<std::string>("_desc", MGE::EMPTY_STRING) )
		);
	}
	
	return false; // we should process other events ...
}
bool MGE::WorldMap::handlePersonelNumChanged(const CEGUI::EventArgs& args) {
	LOG_DEBUG("handlePersonelNumChanged");
	
	CEGUI::Spinner* spinner = static_cast<CEGUI::Spinner*>(static_cast<const CEGUI::WindowEventArgs&>(args).window);
	MGE::BasePrototype* proto = static_cast<MGE::BasePrototype*>(spinner->getUserData());
	
	currentSelectedPersonels[proto] = spinner->getCurrentValue();
	
	LOG_DEBUG(" proto=" << proto->getName() << "  count=" << currentSelectedPersonels[proto]);
	
	int freeSpace = currentSelectedUnit->proto->getPropertyValue<int>("PersonelSpace", 0);
	for (auto& iter : currentSelectedPersonels) {
		freeSpace -= iter.second;
	}
	
	LOG_DEBUG(" free space=" << freeSpace);
	
	if (freeSpace < 0) {
		currentSelectedPersonels[proto] += freeSpace;
		spinner->setCurrentValue(currentSelectedPersonels[proto]);
	}
	
	return true;
}

bool MGE::WorldMap::handleSendNumChanged(const CEGUI::EventArgs& args) {
	if (unitSendNum->getCurrentValue() > 0)
		unitSend->setProperty("Disabled", "False");
	else
		unitSend->setProperty("Disabled", "True");
	return true;
}


bool MGE::WorldMap::handleSend(const CEGUI::EventArgs& args) {
	LOG_DEBUG("handleSend");
	
	int vehicleCount = unitSendNum->getCurrentValue();
	currentSelectedUnit->quantity -= vehicleCount;
	unitSendNum->setProperty("MaximumValue", CEGUI::PropertyHelper<int>::toString(currentSelectedUnit->quantity));
	
	unitsOnTheWay.push_back( new MGE::WorldMap::UnitOnWorldMap(
		currentSelectedUnit->proto, currentSelectedPersonels, vehicleCount, currentSelectedBase
	) );
	currentSelectedPersonels.clear();
	
	baseWin->activate();
	
	return true;
}

bool MGE::WorldMap::handleHideBaseWin(const CEGUI::EventArgs& args) {
	baseWin->hide();
	
	return true;
}


/*  ----====  utils  ====----  */

MGE::WorldMap::BaseOnWorldMap* MGE::WorldMap::findBase(int x, int y) {
	for (auto& iter : bases) {
		if (iter->x == x && iter->y == y) {
			return iter;
		}
	}
	return NULL;
}

MGE::WorldMap::BaseOnWorldMap* MGE::WorldMap::findBase(CEGUI::Window* win) {
	for (auto& iter : bases) {
		LOG_DEBUG("base " << iter->win << " ?= " << win);
		if (iter->win == win) {
			return iter;
		}
	}
	return NULL;
}



/*  ----====                                        ====----  */
/*  ----====  Sub clases of MGE::WorldMap  ====----  */
/*  ----====                                        ====----  */



/*  ----====  SceneGraph  ====----  */

#define PNG_SKIP_SETJMP_CHECK
#include <png.h>

MGE::WorldMap::SceneGraph::SceneGraph(const std::string& mapFile) {
	LOG_INFO("Creating WorldMap::SceneGraph");
	
	int ret = read_png(mapFile.c_str());
	
	if (ret == 0) {
		LOG_INFO(" - read " << roads.size() << " road points");
	} else {
		LOG_WARNING("Unable to load roads from image file: " << mapFile << " error code = " << ret);
	}
}

int MGE::WorldMap::SceneGraph::read_png(const char* file_name) {
	png_structp png_ptr;
	png_infop info_ptr;
	png_uint_32 width, height;
	int bit_depth, color_type, interlace_type;
	unsigned int col, row, skip_cnt, row_len;
	FILE* fp;
	
	fp = fopen(file_name, "rb");
	if (fp == NULL) {
		return E_FOPEN;
	}
	
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		fclose(fp);
		return E_READ_STRUCT;
	}
	
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL || setjmp(png_jmpbuf(png_ptr))) {
		fclose(fp);
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return E_INFO_STRUCT;
	}
	
	png_init_io(png_ptr, fp);
	
	png_read_png(
		png_ptr, info_ptr,
		PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_STRIP_ALPHA | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND, NULL
	);
	
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, NULL, NULL);
	
	if (color_type == PNG_COLOR_TYPE_GRAY) {
		skip_cnt = 1;
	} else if (color_type == PNG_COLOR_TYPE_RGB) {
		skip_cnt = 3;
	} else {
		fclose(fp);
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return E_PNG_TYPE;
	}
	row_len = width * skip_cnt;
	
	png_bytepp row_pointers = png_get_rows(png_ptr, info_ptr);
	for (row=0; row<height; ++row) {
		for (col=0; col<row_len; col=col+skip_cnt) {
			if (row_pointers[row][col] < 50) {
				roads.insert( MICROPATHER_STATE_TYPE(col/skip_cnt, row) );
			}
		}
	}
	
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	fclose(fp);
	return 0;
}

float MGE::WorldMap::SceneGraph::LeastCostEstimate(
	MICROPATHER_STATE_TYPE stateStart,
	MICROPATHER_STATE_TYPE stateEnd
) {
	return Ogre::Math::Sqrt(stateStart.a*stateStart.a + stateEnd.b*stateEnd.b);
}

void MGE::WorldMap::SceneGraph::makeNeighbor(
	MICROPATHER_STATE_TYPE src, int a, int b, float cost,
	std::vector< MGE::MicroPather::StateCost >* states
) {
	src.a = src.a + a;
	src.b = src.b + b;
	
	if ( roads.find(src) != roads.end() ) {
		MGE::MicroPather::StateCost nodeCost = { src , cost };
		states->push_back( nodeCost );
	}
}

void MGE::WorldMap::SceneGraph::AdjacentCost(
	MICROPATHER_STATE_TYPE state,
	std::vector< MGE::MicroPather::StateCost >* states
) {
	makeNeighbor(state, +1,  0,  1.0, states);
	makeNeighbor(state, +1, +1,  1.2, states);
	makeNeighbor(state,  0, +1,  1.0, states);
	makeNeighbor(state, -1, +1,  1.2, states);
	makeNeighbor(state, -1,  0,  1.0, states);
	makeNeighbor(state, -1, -1,  1.2, states);
	makeNeighbor(state,  0, -1,  1.0, states);
	makeNeighbor(state, +1, -1,  1.2, states);
}
	
void MGE::WorldMap::SceneGraph::PrintStateInfo(MICROPATHER_STATE_TYPE state) {
	std::cout << "(" << state.a << "," << state.b << ") ";
}


/*  ----====  BaseOnWorldMap and UnitInBase  ====----  */

MGE::WorldMap::BaseOnWorldMap::BaseOnWorldMap(const pugi::xml_node& xmlNode) {
	x = xmlNode.attribute("x").as_int(0);
	y = xmlNode.attribute("y").as_int(0);
	
	if (MGE::WorldMap::getPtr()->findBase(x, y))
		throw std::logic_error("Base at x=" + std::to_string(x) + " y=" + std::to_string(y) + " exist on World Map");
	
	for (auto xmlSubNode : xmlNode.children("unit")) {
		units.emplace_back( xmlSubNode );
	}
}

bool MGE::WorldMap::BaseOnWorldMap::restoreFromXML(const pugi::xml_node& xmlNode, const MGE::LoadingContext* /*context*/) {
	// restore units status
	units.clear();
	for (auto xmlSubNode : xmlNode.children("unit")) {
		units.emplace_back( xmlSubNode );
	}
	return true;
}

bool MGE::WorldMap::BaseOnWorldMap::storeToXML(pugi::xml_node& xmlNode, bool /*onlyRef*/) const {
	// for base identification we store position on world map (x,y)
	xmlNode.append_attribute("x") << x;
	xmlNode.append_attribute("y") << y;
	
	// we store base units status
	for (const auto& iter : units) {
		auto xmlSubSubNode = xmlNode.append_child("unit");
		iter.storeToXML( xmlSubSubNode, false );
	}
	return true;
}

MGE::WorldMap::UnitInBase::UnitInBase(const pugi::xml_node& xmlNode) {
	proto = MGE::PrototypeFactory::getPtr()->getPrototype(xmlNode);
	quantity = xmlNode.attribute("quantity").as_int(1);
}

bool MGE::WorldMap::UnitInBase::storeToXML(pugi::xml_node& xmlNode, bool /*onlyRef*/) const {
	proto->storeToXML(xmlNode); // store prototype as atrributes
	xmlNode.append_attribute("quantity") = quantity;
	return true;
}


/*  ----====  UnitOnWorldMap  ====----  */

MGE::WorldMap::UnitOnWorldMap::UnitOnWorldMap(
	const MGE::BasePrototype* u,
	const std::unordered_map<MGE::BasePrototype*, int>& p,
	int q,
	MGE::WorldMap::BaseOnWorldMap* b)
{
	proto     = u;
	personel  = p;
	quantity  = q;
	fromBase  = b;
	position  = b->path.size() - 1;
	
	initUnitOnWorldMap();
}

MGE::WorldMap::UnitOnWorldMap::UnitOnWorldMap(const pugi::xml_node& xmlNode) {
	proto = MGE::PrototypeFactory::getPtr()->getPrototype(xmlNode);
	for (auto xmlSubNode : xmlNode.child("personel")) {
		personel.insert(std::make_pair(
			MGE::PrototypeFactory::getPtr()->getPrototype(xmlSubNode.child("Prototype")),
			xmlSubNode.child("quantity").text().as_int()
		));
	}
	quantity = xmlNode.child("quantity").text().as_int();
	position = xmlNode.child("position").text().as_float();
	
	int bx = xmlNode.child("base_x").text().as_int();
	int by = xmlNode.child("base_y").text().as_int();
	fromBase = MGE::WorldMap::getPtr()->findBase( bx, by );
	if (!fromBase)
		throw std::logic_error("Can't find base at x=" + std::to_string(bx) + " y=" + std::to_string(by) );
	
	initUnitOnWorldMap();
}

MGE::WorldMap::UnitOnWorldMap::~UnitOnWorldMap() {
	LOG_DEBUG("destroy UnitOnWorldMap");
	MGE::WorldMap::getPtr()->mapWin->destroyChild(win);
}

void MGE::WorldMap::UnitOnWorldMap::initUnitOnWorldMap() {
	win = CEGUI::WindowManager::getSingleton().loadLayoutFromFile("WorldMap/Marker-Vehicle.layout");
	CEGUI::Window* point = win->getChild( "Point" );
	win->getChild( "Text" )->setText(
		STRING_TO_CEGUI( proto->getPropertyValue("_code", MGE::EMPTY_STRING) )
	);
	
	speed = proto->getPropertyValue<float>("WorldMapSpeed", 1.0);
	
	x_offset = point->getXPosition() + point->getWidth() * 0.5;
	y_offset = point->getYPosition() + point->getHeight() * 0.5;
	update(0);
	
	win->show();
	MGE::WorldMap::getPtr()->mapWin->addChild(win);
}

void MGE::WorldMap::UnitOnWorldMap::update(float gameTimeStep) {
	position = position - gameTimeStep * speed;
	int pos = position;
	
	win->setPosition(CEGUI::UVector2(
		CEGUI::UDim( static_cast<float>(fromBase->path[pos].a)/MGE::WorldMap::getPtr()->mapWidth,  0) - x_offset,
		CEGUI::UDim( static_cast<float>(fromBase->path[pos].b)/MGE::WorldMap::getPtr()->mapHeight, 0) - y_offset
	));
}

bool MGE::WorldMap::UnitOnWorldMap::storeToXML(pugi::xml_node& xmlNode, bool /*onlyRef*/) const {
	xmlNode.append_child("position") << position;
	xmlNode.append_child("quantity") << quantity;
	
	xmlNode.append_child("base_x") << fromBase->x;
	xmlNode.append_child("base_y") << fromBase->y;
	
	proto->storeToXML(xmlNode.append_child("Prototype")); // store prototype as child node
	
	auto xmlSubNode = xmlNode.append_child("personel");
	for (const auto& iter : personel) {
		auto xmlSubSubNode = xmlSubNode.append_child("person");
		xmlSubSubNode.append_child("quantity") << iter.second;
		iter.first->storeToXML(xmlSubSubNode);
	}
	return true;
}
