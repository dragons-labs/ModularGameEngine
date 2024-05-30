/*
Copyright (c) 2013-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>
Copyright (c) 2008-2013 Ismail TARIM <ismail@royalspor.com> and the Ogitor Team

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

Based on:
	→ public domain code from Ogre Wiki
		- ex http://www.ogre3d.org/tikiwiki/tiki-index.php?page=RapidXML+Dotscene+Loader
		- now https://web.archive.org/web/20151124194207/http://www.ogre3d.org/tikiwiki/tiki-index.php?page=RapidXML+Dotsceneloader-cpp&structure=Cookbook
	→ MIT licensed code from OGITOR
		- ex http://www.ogitor.org
		- now https://github.com/OGRECave/ogitor
*/

#include "data/DotSceneLoader.h"

#include "with.h"
#include "LogSystem.h"
#include "SceneLoader.h"
#include "ConfigParser.h"
#include "XmlUtils.h"
#include "data/property/XmlUtils_Ogre.h"
#include "data/utils/OgreResources.h"
#include "data/utils/NamedSceneNodes.h"
#include "data/utils/OgreSceneObjectInfo.h"
#include "rendering/utils/RenderQueueGroups.h"
#include "rendering/utils/VisibilityFlags.h"
#include "data/QueryFlags.h"
#include "data/utils/OgreUtils.h"


#include <OgreCommon.h>
#include <OgreSceneNode.h>
#include <OgreMovableObject.h>
#include <OgreSceneManager.h>
#include <OgreMeshManager.h>
#include <OgreItem.h>
#include <OgreEntity.h>
#include <OgreSubEntity.h>
#include <OgreBillboardSet.h>
#include <OgreRoot.h>
#include <OgreHlmsManager.h>
#include <OgreMaterialManager.h>

#define WITH_XML_NOT_EMPTY(value) WITH_NOT_NULL(value, WITH_AS)

MGE_REGISTER_MODULE(environment, MGE::DotSceneLoader::processEnvironment);
MGE_REGISTER_SCENE_ELEMENT_CAST(subSceneFile,    MGE::DotSceneLoader::processSceneFile);
MGE_REGISTER_SCENE_ELEMENT_CAST(node,            MGE::DotSceneLoader::processNode);
MGE_REGISTER_SCENE_ELEMENT_CAST(item,            MGE::DotSceneLoader::processItem);
MGE_REGISTER_SCENE_ELEMENT_CAST(entity,          MGE::DotSceneLoader::processEntity);
MGE_REGISTER_SCENE_ELEMENT_CAST(light,           MGE::DotSceneLoader::processLight);
MGE_REGISTER_SCENE_ELEMENT_CAST(particleSystem,  MGE::DotSceneLoader::processParticleSystem);
MGE_REGISTER_SCENE_ELEMENT_CAST(billboardSet,    MGE::DotSceneLoader::processBillboardSet);
MGE_REGISTER_SCENE_ELEMENT_CAST(lookTarget,      MGE::DotSceneLoader::processLookTarget);
MGE_REGISTER_SCENE_ELEMENT_CAST(trackTarget,     MGE::DotSceneLoader::processTrackTarget);
//MGE_REGISTER_SCENE_ELEMENT_CAST(plane,           MGE::DotSceneLoader::processPlane);


/**
@page XMLSyntax_SceneConfig

@subsection XMLNode_SubSceneFile \<subSceneFile\>

`<subSceneFile>` is used for include next .scene file (subSceneFile) and has the following attributes:
  - @c path  path to file to include
  - @c name  name of file to include to get from resources (used only when path not set or empty)
  - @c group group name for getting file from resources (default "MapsConfigs")

Sub scene files use standard `.scene.xml` files syntax (root XML node must be `<scene>`),
but the only one supported subnode is @ref XMLNode_Nodes (all other subnodes of `<scene>` will be silently ignored).
*/
void* MGE::DotSceneLoader::processSceneFile(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context, const MGE::SceneObjectInfo& parent) {
	auto filePath = MGE::OgreResources::getResourcePath( xmlNode, "MapsConfigs" );
	
	if (filePath.empty()) {
		LOG_WARNING("Can't find file to include as sub-scene");
		return nullptr;
	}
	
	pugi::xml_document* xmlFile = new pugi::xml_document();
	auto xmlRootNode = MGE::XMLUtils::openXMLFile(*xmlFile, filePath.c_str(), "scene");	
	
	MGE::LoadingContext subFileContext(*context);
	subFileContext.linkToXML = false;
	
	for (auto xmlSubNode : xmlRootNode.children("nodes")) {
		MGE::SceneLoader::getPtr()->parseSceneXMLNode( xmlSubNode, &subFileContext, parent );
	}
	return nullptr;
}

//
// environment procesing function
//

/**
@page XMLSyntax_MapAndSceneConfig

@subsection XMLNode_Environment \<environment\>

@c \<environment\> can be used for:
  - scene lighting via:
    - @c \<ambientLight\> tag (for Ogre \> 2.0) with subnodes:
      - @c \<upperHemisphere\>
        - @ref XML_ColourValue
        - colour when the surface normal is close to hemisphereDir
      - @c \<lowerHemisphere\>
        - @ref XML_ColourValue
        - colour when the surface normal is pointing away from hemisphereDir
      - @c \<hemisphereDir\>
        - @ref XML_Vector3
        - hemisphere's direction reference to compare the surface normal to. (vector will be normalized, default up to Y axis)
      - @c \<envmapScale\>
        - floating point
        - global scale to apply to all environment maps (for relevant Hlms implementations, like PBS)
  - fog via @c \<fog\> tag with attributes:
    - @c density
      - floating point
      - value for fog density
    - @c start
      - floating point
      - value for start fog distance
    - @c end
      - floating point
      - value for end fog distance
    - @c mode     it can be one of the following string:
      - @c none   for no fog
      - @c exp    for fog density increases exponentially from the camera (fog = 1/e^(distance * density)) 
      - @c exp2   for fog density increases at the square of FOG_EXP, i.e. even quicker (fog = 1/e^(distance * density)^2)
      - @c linear for fog density increases linearly between the start and end distances.
    .
    and subnodes:
    - @c \<colour\> with ColourValue for fog colour.
  - sky compositor postprocessing material (only Ogre \> 2.0) dome via @c \<sky\> tag with attributes:
    - @c material
      - name of material used for sky (Warning: "SkyPostprocess" material used in compositor will be override by this material)
*/
MGE::Module* MGE::DotSceneLoader::processEnvironment(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context) {
	LOG_INFO("processEnvironment");
	
	WITH_XML_NOT_EMPTY(xmlNode.child("ambientLight")) {
		context->scnMgr->setAmbientLight(
			MGE::XMLUtils::getValue<Ogre::ColourValue>(WITH_AS.child("upperHemisphere"), Ogre::ColourValue::Black),
			MGE::XMLUtils::getValue<Ogre::ColourValue>(WITH_AS.child("lowerHemisphere"), Ogre::ColourValue::Black),
			MGE::XMLUtils::getValue<Ogre::Vector3>(WITH_AS.child("hemisphereDir"), Ogre::Vector3::UNIT_Y),
			WITH_AS.child("envmapScale").text().as_float(1.0)
		);
	}
	
	WITH_XML_NOT_EMPTY(xmlNode.child("fog")) {
		// Process attributes
		Ogre::Real expDensity  = WITH_AS.attribute("density").as_float(0.001);
		Ogre::Real linearStart = WITH_AS.attribute("start").as_float(0.0);
		Ogre::Real linearEnd   = WITH_AS.attribute("end").as_float(1.0);
		
		Ogre::FogMode mode;
		std::string_view sMode = WITH_AS.attribute("mode").as_string();
		if(sMode == "none")
			mode = Ogre::FOG_NONE;
		else if(sMode == "exp")
			mode = Ogre::FOG_EXP;
		else if(sMode == "exp2")
			mode = Ogre::FOG_EXP2;
		else if(sMode == "linear")
			mode = Ogre::FOG_LINEAR;
		else
			mode = static_cast<Ogre::FogMode>(MGE::StringUtils::toNumeric<int>(sMode, 0));
		
		Ogre::ColourValue colourDiffuse = Ogre::ColourValue::White;
		auto xmlTagNode = WITH_AS.child("colour");
		if(xmlTagNode)
			colourDiffuse = MGE::XMLUtils::getValue<Ogre::ColourValue>(xmlTagNode);
		
		// Setup the fog
		context->scnMgr->setFog(mode, colourDiffuse, expDensity, linearStart, linearEnd);
	}
	
	WITH_XML_NOT_EMPTY(xmlNode.child("sky")) {
		std::string material  = WITH_AS.attribute("material").as_string();
		
		auto skyMatererial = Ogre::MaterialManager::getSingletonPtr()->getByName(material);
		if (!skyMatererial)
			throw std::logic_error("Can't find material " + material + " for sky");
		
		LOG_INFO("set sky with material=" + material);
		
		auto skyDstMatererial = Ogre::MaterialManager::getSingletonPtr()->getByName("SkyPostprocess");
		if (skyDstMatererial) {
			skyMatererial->copyDetailsTo(skyDstMatererial);
		} else {
			skyMatererial->clone("SkyPostprocess");
		}
	}
	
	/// @todo TODO.6: remove this dead (Ogre < 2.3) code ?
	#if 0 // not supported in Ogre >= 2.2
	xmlTagNode = xmlNode.child("skyDome");
	if(xmlTagNode) {
		if(! xmlTagNode.attribute("active").as_bool(true) )
			return;
		
		MGE::Utils::String material  = xmlTagNode.attribute("material").as_string();
		Ogre::Real         distance  = xmlTagNode.attribute("distance").as_float(5000);
		bool               drawFirst = xmlTagNode.attribute("drawFirst").as_bool(true);
		Ogre::Quaternion   rotation  = MGE::XMLUtils::getValue<Ogre::Quaternion>(xmlTagNode.child("rotation"), Ogre::Quaternion::IDENTITY);
		Ogre::Real         curvature = xmlTagNode.attribute("curvature").as_float(10);
		Ogre::Real         tiling    = xmlTagNode.attribute("tiling").as_float(8);
		
		LOG_INFO("set skyDome with material=" + material + " curvature=" + curvature + " tiling=" + tiling);
		context->scnMgr->setSkyDome(true, material.getObjStr<Ogre::String>(), curvature, tiling, distance, drawFirst, rotation, 16, 16, -1);
	}
	
	xmlTagNode = xmlNode.child("skyBox");
	if(xmlTagNode) {
		if(! xmlTagNode.attribute("active").as_bool(true) )
			return;
		
		MGE::Utils::String material  = xmlTagNode.attribute("material").as_string();
		Ogre::Real         distance  = xmlTagNode.attribute("distance").as_float(5000);
		bool               drawFirst = xmlTagNode.attribute("drawFirst").as_bool(true);
		Ogre::Quaternion   rotation  = MGE::XMLUtils::getValue<Ogre::Quaternion>(xmlTagNode.child("rotation"), Ogre::Quaternion::IDENTITY);
		
		LOG_INFO("set skyBox with material=" + material);
		context->scnMgr->setSkyBox(true, material.getObjStr<Ogre::String>(), distance, drawFirst, rotation);
	}
	
	xmlTagNode = xmlNode.child("skyPlane");
	if(xmlTagNode) {
		if(! xmlTagNode.attribute("active").as_bool(true) )
			return;
		
		MGE::Utils::String material  = xmlTagNode.attribute("material").as_string();
		bool               drawFirst = xmlTagNode.attribute("drawFirst").as_bool(true);
		Ogre::Real         planeX    = xmlTagNode.attribute("planeX").as_float();
		Ogre::Real         planeY    = xmlTagNode.attribute("planeY").as_float(-1);
		Ogre::Real         planeZ    = xmlTagNode.attribute("planeZ").as_float();
		Ogre::Real         planeD    = xmlTagNode.attribute("planeD").as_float(5000);
		Ogre::Real         scale     = xmlTagNode.attribute("scale").as_float(1000);
		Ogre::Real         bow       = xmlTagNode.attribute("bow").as_float();
		Ogre::Real         tiling    = xmlTagNode.attribute("tiling").as_float(10);
		
		Ogre::Plane plane;
		plane.normal = Ogre::Vector3(planeX, planeY, planeZ);
		plane.d = planeD;
		
		LOG_INFO("set skyPlane with material=" + material + " tiling=" + tiling);
		context->scnMgr->setSkyPlane(true, plane, material.getObjStr<Ogre::String>(), scale, tiling, drawFirst, bow, 1, 1);
	}
	#endif
	
	return reinterpret_cast<MGE::Module*>(1);
}


//
//  scene main sub-elements procesing function
//

/**
@page XMLSyntax_SceneConfig

@subsection XMLNode_Node \<node\>

@c \<node\> is used for describe Ogre Scene Node and has the following attributes:
  - @c name (optional, when not empty create named scene node - set name to scene node and register in named scene nodes map (MGE::LoadingSystem::sceneNodes))
  .
  and following subnodes:
  - @c \<position\>
    - @ref XML_Vector3
  - @c \<rotation\>
    - @ref XML_Quaternion
  - @c \<scale\>
    - @ref XML_Vector3
  - any other xml nodes register for procesing @ref XMLNode_Nodes in .scene files (including @c \<item\>, @c \<node\> or @c \<subSceneFile\> elements)
*/
Ogre::SceneNode* MGE::DotSceneLoader::processNode(
	const pugi::xml_node& xmlNode, const MGE::LoadingContext* context, const MGE::SceneObjectInfo& parent
) {
	std::string_view  name     = xmlNode.attribute("name").as_string();
	Ogre::Vector3     position = MGE::XMLUtils::getValue(xmlNode.child("position"), Ogre::Vector3::ZERO);
	Ogre::Quaternion  rotation = MGE::XMLUtils::getValue(xmlNode.child("rotation"), Ogre::Quaternion::IDENTITY);
	Ogre::Vector3     scale    = MGE::XMLUtils::getValue(xmlNode.child("scale"), Ogre::Vector3::UNIT_SCALE);
	
	LOG_INFO("create scene node with name=" << name << "  position=" << position << " rotation=" << rotation << " scale=" << scale);
	
	Ogre::SceneNode* ogreNode = MGE::NamedSceneNodes::createSceneNode(name, parent.node, Ogre::SCENE_DYNAMIC, position, rotation, scale);
	if (!ogreNode)
		return NULL;
	
	if (context->linkToXML) {
		ogreNode->getUserObjectBindings().setUserAny("xml", Ogre::Any(xmlNode));
	}
	MGE::SceneLoader::getPtr()->parseSceneXMLNode(xmlNode, context, {ogreNode, NULL});
	
	return ogreNode;
}

/**
@page XMLSyntax_SceneConfig

@subsection XMLNode_Item \<item\>

@c \<item\> is used for describe Ogre V2 objects (Items) and has the following attributes:
  - @c name
    - optional
    - when not empty create named element - set name to item)
  - @c meshFile
    - name of mesh to load
  - @c meshResourceGroup
    - resource group for meshFile (if not set use @ref MGE::LoadingContext::defaultResourceGroup)
  - @c materialName
    - name of material (HLMS datablock) to use (default material selected by mesh)
  - @c materialResourceGroup
    - resource group for materialName (if not set use @ref MGE::LoadingContext::defaultResourceGroup)
  - @c static
    - @ref XML_Bool
    - when true create as SCENE_STATIC (otherwise @c SCENE_DYNAMIC)
  - @c castShadows
    - @ref XML_Bool
    - default true
  - @c isGround
    - @ref XML_Bool
    - default false
    - when true set only @c GROUND query flag (otherwise check @c isVisualOnly and set @c OGRE_OBJECT / @c COLLISION_OBJECT)
  - @c isVisualOnly
    - @ref XML_Bool
    - default false
    - when true set only @c OGRE_OBJECT query flag (otherwise set @c OGRE_OBJECT and @c COLLISION_OBJECT)
  - @c renderQueueGroups
    - numeric value or string
    - string parsed with @ref MGE::RenderQueueGroups::fromString
    - strings is literally identical to @ref MGE::RenderQueueGroups enum elements names WITHOUT _V1, _V2 surfix
    - will be used *_V2 values
  - @c visibilityFlag
    - numeric value or string
    - see @ref MGE::VisibilityFlags
    - string parsed with @ref MGE::VisibilityFlags::fromString and @ref MGE::StringUtils::stringToNumericMask – can be a space delimited list of flags
    - strings is literally identical to @ref MGE::VisibilityFlags (namespace enum) elements names
  .
  and following subnodes:
  - @c \<subentity\> to set material for subitem (can be used multiple times), has the following attributes:
    - @c index
      - number (id) of subitem to set material
    - @c materialName
      - name of material to set
  - any other xml nodes register for procesing @ref XMLNode_Nodes in .scene files (but you should not use some xml elements inside @c \<item\> elememnt, e.g. @c \<node\> elements)
*/
Ogre::Item* MGE::DotSceneLoader::processItem(
	const pugi::xml_node& xmlNode, const MGE::LoadingContext* context, const MGE::SceneObjectInfo& parent
) {
	// Process attributes
	auto name = xmlNode.attribute("name").as_string();
	auto meshFile = xmlNode.attribute("meshFile").as_string();
	auto materialName = xmlNode.attribute("materialName").as_string();
	auto meshResourceGroup = xmlNode.attribute("meshResourceGroup").as_string(context->defaultResourceGroup.c_str());
	auto materialResourceGroup = xmlNode.attribute("materialResourceGroup").as_string(context->defaultResourceGroup.c_str());
	bool castShadows = xmlNode.attribute("castShadows").as_bool(true);
	
	Ogre::Item* pItem = 0;
	try {
		Ogre::SceneMemoryMgrTypes sceneType = Ogre::SCENE_DYNAMIC;
		if (xmlNode.attribute("static").as_bool(false))
			sceneType = Ogre::SCENE_STATIC;
		
		pItem = context->scnMgr->createItem(
			meshFile,
			meshResourceGroup,
			sceneType
		);
		
		pItem->setRenderQueueGroup(
			MGE::RenderQueueGroups::fromString(
				xmlNode.attribute("renderQueueGroups").as_string("DEFAULT")
			 )
		);
		if (xmlNode.attribute("visibilityFlag")) {
			pItem->setVisibilityFlags(
				MGE::StringUtils::stringToNumericMask<uint32_t>(
					xmlNode.attribute("visibilityFlag").as_string(),
					&MGE::VisibilityFlags::fromString
				)
			);
		}
		pItem->setCastShadows(castShadows);
		
		if (materialName[0]) {
			LOG_VERBOSE("set material for item: " << materialName);
			pItem->setDatablockOrMaterialName(
				materialName,
				materialResourceGroup
			);
		}
		
		// process SubItems
		auto xmlTagNode = xmlNode.child("subitem");
		while (xmlTagNode) {
			int sIndex = xmlTagNode.attribute("index").as_int(-1); // subitem index
			materialName = xmlTagNode.attribute("materialName").as_string();       // new material for subitem
			if (sIndex >= 0 && materialName[0]) {
				try {
					LOG_VERBOSE("set material for subitem: " << materialName);
					auto subItem = pItem->getSubItem( sIndex );
					subItem->setDatablockOrMaterialName(
						materialName,
						materialResourceGroup
					);
				} catch (...) {
					LOG_WARNING("DotSceneLoader: subitem material index invalid!");
				}
			}
			xmlTagNode = xmlTagNode.next_sibling("subitem");
		}
	} catch(Ogre::Exception& e) {
		LOG_ERROR("DotSceneLoader: item " << name << "loading error: " << e.getDescription());
		return pItem;
	}
	
	if (name[0])
		pItem->setName(name);
	
	parent.node->attachObject(pItem);
	
	if (xmlNode.attribute("isGround").as_bool(false)) {
		pItem->setQueryFlags( MGE::QueryFlags::GROUND );
	} else if (xmlNode.attribute("isVisualOnly").as_bool(false)) {
		pItem->setQueryFlags( MGE::QueryFlags::OGRE_OBJECT );
	} else {
		pItem->setQueryFlags( MGE::QueryFlags::OGRE_OBJECT | MGE::QueryFlags::COLLISION_OBJECT );
	}
	
	MGE::SceneLoader::getPtr()->parseSceneXMLNode(xmlNode, context, {parent.node, pItem});
	
	return pItem;
}

/**
@page XMLSyntax_SceneConfig

@subsection XMLNode_Entity \<entity\>

@c \<entity\> is used for describe Ogre V1 objects (Entity) and has the following attributes:
  - @c name
    - optional
    - when not empty create named element (set name to entity)
  - @c meshFile
    - name of mesh to load
  - @c meshResourceGroup
    - resource group for meshFile (if not set use @ref MGE::LoadingContext::defaultResourceGroup)
  - @c materialName
    - name of material (or HLMS datablock) to use (default material selected by mesh)
  - @c materialResourceGroup
    - resource group for materialName (if not set use @ref MGE::LoadingContext::defaultResourceGroup)
  - @c castShadows
    - @ref XML_Bool
    - default true
  - @c isGround
    - @ref XML_Bool
    - default false
    - when true set only @c GROUND query flag (otherwise check @c isVisualOnly and set @c OGRE_OBJECT / @c COLLISION_OBJECT)
  - @c isVisualOnly
    - @ref XML_Bool
    - default false
    - when true set only @c OGRE_OBJECT query flag (otherwise set @c OGRE_OBJECT and @c COLLISION_OBJECT)
  - @c renderQueueGroups
    - numeric value or string
    - string parsed with @ref MGE::RenderQueueGroups::fromString
    - strings is literally identical to @ref MGE::RenderQueueGroups enum elements names WITHOUT _V1, _V2 surfix
    - will be used *_V1 values
  - @c visibilityFlag
    - numeric value or string
    - string parsed with @ref MGE::VisibilityFlags::fromString and @ref MGE::StringUtils::stringToNumericMask – can be a space delimited list of flags
    - strings is literally identical to @ref MGE::VisibilityFlags (namespace enum) elements names
  .
  and following subnodes:
  - @c \<subentity\> to set material for subentity (can be used multiple times), has the following attributes:
    - @c index
      - number (id) of subentity to set material
    - @c materialName
      - name of material to set
  - any other xml nodes register for procesing @ref XMLNode_Nodes in .scene files (but you should not use some xml elements inside @c \<item\> elememnt, e.g. @c \<node\> elements)
*/
Ogre::v1::Entity* MGE::DotSceneLoader::processEntity(
	const pugi::xml_node& xmlNode, const MGE::LoadingContext* context, const MGE::SceneObjectInfo& parent
) {
	// Process attributes
	auto name = xmlNode.attribute("name").as_string();
	auto meshFile = xmlNode.attribute("meshFile").as_string();
	auto materialName = xmlNode.attribute("materialName").as_string();
	auto meshResourceGroup = xmlNode.attribute("meshResourceGroup").as_string(context->defaultResourceGroup.c_str());
	auto materialResourceGroup = xmlNode.attribute("materialResourceGroup").as_string(context->defaultResourceGroup.c_str());
	bool castShadows = xmlNode.attribute("castShadows").as_bool(true);
	
	/*
	// Process vertexBuffer
	xmlTagNode = xmlNode.child("vertexBuffer");
	if(xmlTagNode)
		processVertexBuffer(xmlTagNode);
	
	// Process indexBuffer
	xmlTagNode = xmlNode.child("indexBuffer");
	if(xmlTagNode)
		processIndexBuffer(xmlTagNode);
	*/
	
	// Create the entity
	Ogre::v1::Entity* pEntity = 0;
	try {
		Ogre::v1::MeshManager::getSingleton().load(
			meshFile,
			meshResourceGroup,
			Ogre::v1::HardwareBuffer::HBU_STATIC,
			Ogre::v1::HardwareBuffer::HBU_STATIC
		);
		
		pEntity = context->scnMgr->createEntity(meshFile);
		if (name[0])
			pEntity->setName(name);
		
		pEntity->setRenderQueueGroup(
			MGE::RenderQueueGroups::fromString(
				xmlNode.attribute("renderQueueGroups").as_string("DEFAULT"),
				true
			 )
		);
		if (xmlNode.attribute("visibilityFlag")) {
			pEntity->setVisibilityFlags(
				MGE::StringUtils::stringToNumericMask<uint32_t>(
					xmlNode.attribute("visibilityFlag").as_string(),
					&MGE::VisibilityFlags::fromString
				)
			);
		}
		pEntity->setCastShadows(castShadows);
		parent.node->attachObject(pEntity);
		
		if(materialName[0]) {
			LOG_VERBOSE("set material for entity: " << materialName);
			pEntity->setDatablockOrMaterialName(
				materialName,
				materialResourceGroup
			);
		}
		
		// process SubEntity
		auto xmlTagNode = xmlNode.child("subentity");
		while (xmlTagNode) {
			int sIndex = xmlTagNode.attribute("index").as_int(-1); // submesh index
			materialName = xmlTagNode.attribute("materialName").as_string();       // new material for submesh
			if (sIndex >=0 && materialName[0]) {
				try {
					LOG_VERBOSE("set material for subentity: " << materialName);
					auto subEntity = pEntity->getSubEntity(sIndex);
					subEntity->setDatablockOrMaterialName(
						materialName,
						materialResourceGroup
					);
				} catch (...) {
					LOG_WARNING("DotSceneLoader: subentity material index invalid!");
				}
			}
			xmlTagNode = xmlTagNode.next_sibling("subentity");
		}
	} catch(Ogre::Exception& e) {
		LOG_ERROR("DotSceneLoader: error loading an entity: " << e.getDescription());
		return pEntity;
	}
	
	if (xmlNode.attribute("isGround").as_bool(false)) {
		pEntity->setQueryFlags( MGE::QueryFlags::GROUND );
	} else if (xmlNode.attribute("isVisualOnly").as_bool(false)) {
		pEntity->setQueryFlags( MGE::QueryFlags::OGRE_OBJECT );
	} else {
		pEntity->setQueryFlags( MGE::QueryFlags::OGRE_OBJECT | MGE::QueryFlags::COLLISION_OBJECT );
	}
	
	MGE::SceneLoader::getPtr()->parseSceneXMLNode(xmlNode, context, {parent.node, pEntity});
	
	return pEntity;
}

/**
@page XMLSyntax_SceneConfig

@subsection XMLNode_Light \<light\>

@c \<light\> is used for lights in scene and has the following attributes:
  - @c type one of the following string value:
    - @c point
    - @c directional
    - @c spot
  - @c powerScale
    - floting point value
  - @c castShadows
    - @ref XML_Bool
    - default false
  - @c visible
    - @ref XML_Bool
    - default true
  .
  and following subnodes:
  - @c \<direction\> (@c \<directionVector\> and @c \<normal\> do the same)
    - @ref XML_Vector3
    - Warning: affect node orientation (so spot light should have own sub scene node ...)
  - @c \<colourDiffuse\>
    - @ref XML_ColourValue
  - @c \<colourSpecular\>
    - @ref XML_ColourValue
  - @c \<colour\>
    - @ref XML_ColourValue
    - shortcut for set Diffuse and Specular color to this same value
  - @c \<spotlightRange\> (@c \<lightRange\> do the same)
    - has attributes:
      - @c inner
        - floting point value of angle in degrees
      - @c outer
        - floting point value of angle in degrees
      - @c falloff
        - floting point value
  - @c \<lightAttenuation\>
    - set light attenuation, has one of following attributes sets:
      - for set via setAttenuation():
        - @c range
          - floting point value
        - @c constant
          - floting point value
        - @c linear
          - floting point value
        - @c quadratic
          - floting point value
      - for set via setAttenuationBasedOnRadius():
        - @c radius
          - floting point value
        - @c lumThreshold
          - floting point value
*/
Ogre::Light* MGE::DotSceneLoader::processLight(
	const pugi::xml_node& xmlNode, const MGE::LoadingContext* context, const MGE::SceneObjectInfo& parent
) {
	// Create the light
	Ogre::Light* pLight = context->scnMgr->createLight();
	parent.node->attachObject(pLight);
	
	std::string_view sValue = xmlNode.attribute("type").as_string();
	if(sValue == "point")
		pLight->setType(Ogre::Light::LT_POINT);
	else if(sValue == "spot")
		pLight->setType(Ogre::Light::LT_SPOTLIGHT);
	else if(sValue == "directional")
		pLight->setType(Ogre::Light::LT_DIRECTIONAL);
	else {
		LOG_WARNING("Unknow light type: " + sValue);
		return NULL;
	}
	
	pLight->setVisible(xmlNode.attribute("visible").as_bool(true));
	pLight->setCastShadows(xmlNode.attribute("castShadows").as_bool(false));
	pLight->setPowerScale(xmlNode.attribute("powerScale").as_float(1.0));
	
	pugi::xml_node xmlTagNode;
	
	// Process direction / directionVector / normal
	xmlTagNode = xmlNode.child("direction");
	if(!xmlTagNode)  xmlTagNode = xmlNode.child("directionVector");
	if(!xmlTagNode)  xmlTagNode = xmlNode.child("normal");
	if(xmlTagNode)
		pLight->setDirection( MGE::XMLUtils::getValue<Ogre::Vector3>(xmlTagNode).normalisedCopy() );
	
	// Process colourDiffuse
	xmlTagNode = xmlNode.child("colourDiffuse");
	if(!xmlTagNode)  xmlTagNode = xmlNode.child("colour");
	if(xmlTagNode)
		pLight->setDiffuseColour(MGE::XMLUtils::getValue<Ogre::ColourValue>(xmlTagNode));
	
	// Process colourSpecular
	xmlTagNode = xmlNode.child("colourSpecular");
	if(!xmlTagNode)  xmlTagNode = xmlNode.child("colour");
	if(xmlTagNode)
		pLight->setSpecularColour(MGE::XMLUtils::getValue<Ogre::ColourValue>(xmlTagNode));
	
	// Process lightRange
	xmlTagNode = xmlNode.child("spotlightRange");
	if(!xmlTagNode)  xmlTagNode = xmlNode.child("lightRange");
	if(xmlTagNode) {
		Ogre::Real inner = xmlTagNode.attribute("inner").as_float();
		pLight->setSpotlightRange(
			Ogre::Degree( inner ),
			Ogre::Degree( xmlTagNode.attribute("outer").as_float(inner) ),
			xmlTagNode.attribute("falloff").as_float(1.0)
		);
	}
	
	// Process lightAttenuation
	xmlTagNode = xmlNode.child("lightAttenuation");
	if(xmlTagNode){
		auto attrib = xmlTagNode.attribute("radius");
		if (attrib) {
			pLight->setAttenuationBasedOnRadius(
				xmlTagNode.attribute("radius").as_float(),
				xmlTagNode.attribute("lumThreshold").as_float()
			);
		} else {
			pLight->setAttenuation(
				xmlTagNode.attribute("range").as_float(),
				xmlTagNode.attribute("constant").as_float(),
				xmlTagNode.attribute("linear").as_float(),
				xmlTagNode.attribute("quadratic").as_float()
			);
		}
	}
	
	return pLight;
}

#include <OgreParticleSystem.h>

/**
@page XMLSyntax_SceneConfig

@subsection XMLNode_ParticleSystem \<particleSystem\>

@c \<particleSystem\> is used for particles in scene and has the following attributes:
  - @c file
  - @c renderQueueGroups
    - see @ref MGE::RenderQueueGroups
*/
Ogre::ParticleSystem* MGE::DotSceneLoader::processParticleSystem(
	const pugi::xml_node& xmlNode, const MGE::LoadingContext* context, const MGE::SceneObjectInfo& parent
) {
	try {
		Ogre::ParticleSystem* pParticles = context->scnMgr->createParticleSystem( 
			xmlNode.attribute("file").as_string()
		);
		pParticles->setRenderQueueGroup(
			MGE::RenderQueueGroups::fromString(
				xmlNode.attribute("renderQueueGroups").as_string("DEFAULT"),
				true
			 )
		);
		parent.node->attachObject(pParticles);
		return pParticles;
	} catch(Ogre::Exception& e) {
		LOG_ERROR("DotSceneLoader: error creating a particle system: " << e.getDescription());
		return NULL;
	}
}

/**
@page XMLSyntax_SceneConfig

@subsection XMLNode_BillboardSet \<billboardSet\>

@c \<billboardSet\> is used for billboards in scene and has the following attributes:
  - @c name
  - @c poolSize
  - @c type one of the following string value:
    - @c POINT
    - @c ORIENTED_COMMON
    - @c ORIENTED_SELF
    - @c PERPENDICULAR_COMMON
    - @c PERPENDICULAR_SELF
  - @c origin one of the following string value:
    - @c TOP_LEFT
    - @c TOP_CENTER
    - @c TOP_RIGHT
    - @c CENTER_LEFT
    - @c CENTER
    - @c CENTER_RIGHT
    - @c BOTTOM_LEFT
    - @c BOTTOM_CENTER
    - @c BOTTOM_RIGHT
  - @c inWorldSpace
    - @ref XML_Bool
    - default true
  - @c renderQueueGroups
    - see @ref MGE::RenderQueueGroups, use *_V1 values
  - @c material - name of datablock for billboard
  - @c width
  - @c height
  .
  and following subnodes:
  - @c commonDirection
    - @ref XML_Vector3
  - @c commonUpVector
    - @ref XML_Vector3
  - @c offset - billboard offset from parent scene node
    - @ref XML_Vector3
*/
Ogre::v1::BillboardSet* MGE::DotSceneLoader::processBillboardSet(
	const pugi::xml_node& xmlNode, const MGE::LoadingContext* context, const MGE::SceneObjectInfo& parent
) {
	Ogre::v1::BillboardSet* billboardSet = context->scnMgr->createBillboardSet(
		xmlNode.attribute("poolSize").as_int(1)
	);
	
	billboardSet->setName(
		xmlNode.attribute("name").as_string()
	);
	
	std::string_view sValue = xmlNode.attribute("type").as_string();
	if(sValue == "POINT") {
		billboardSet->setBillboardType(Ogre::v1::BBT_POINT);
	} else if(sValue == "ORIENTED_COMMON") {
		billboardSet->setBillboardType(Ogre::v1::BBT_ORIENTED_COMMON);
	} else if(sValue == "ORIENTED_SELF") {
		billboardSet->setBillboardType(Ogre::v1::BBT_ORIENTED_SELF);
	} else if(sValue == "PERPENDICULAR_COMMON") {
		billboardSet->setBillboardType(Ogre::v1::BBT_PERPENDICULAR_COMMON);
	} else if(sValue == "PERPENDICULAR_SELF") {
		billboardSet->setBillboardType(Ogre::v1::BBT_PERPENDICULAR_SELF);
	} else {
		LOG_WARNING("Unknow billboard type: " + sValue);
		return billboardSet;
	}
	
	pugi::xml_node xmlTagNode;
	xmlTagNode = xmlNode.child("commonDirection");
	if (xmlTagNode) {
		// set direction (orthogonal vector to billboard plane)
		billboardSet->setCommonDirection( MGE::XMLUtils::getValue<Ogre::Vector3>(xmlTagNode) );
	}
	xmlTagNode = xmlNode.child("commonUpVector");
	if (xmlTagNode) {
		// set billboard plane up vector
		billboardSet->setCommonUpVector( MGE::XMLUtils::getValue<Ogre::Vector3>(xmlTagNode) );
	}
	
	sValue = xmlNode.attribute("origin").as_string();
	if (!sValue.empty()) {
		if (sValue == "TOP_LEFT") {
			billboardSet->setBillboardOrigin(Ogre::v1::BBO_TOP_LEFT);
		} else if (sValue == "TOP_CENTER") {
			billboardSet->setBillboardOrigin(Ogre::v1::BBO_TOP_CENTER);
		} else if (sValue == "TOP_RIGHT") {
			billboardSet->setBillboardOrigin(Ogre::v1::BBO_TOP_RIGHT);
		} else if (sValue == "CENTER_LEFT") {
			billboardSet->setBillboardOrigin(Ogre::v1::BBO_CENTER_LEFT);
		} else if (sValue == "CENTER") {
			billboardSet->setBillboardOrigin(Ogre::v1::BBO_CENTER);
		} else if (sValue == "CENTER_RIGHT") {
			billboardSet->setBillboardOrigin(Ogre::v1::BBO_CENTER_RIGHT);
		} else if (sValue == "BOTTOM_LEFT") {
			billboardSet->setBillboardOrigin(Ogre::v1::BBO_BOTTOM_LEFT);
		} else if (sValue == "BOTTOM_CENTER") {
			billboardSet->setBillboardOrigin(Ogre::v1::BBO_BOTTOM_CENTER);
		} else if (sValue == "BOTTOM_RIGHT") {
			billboardSet->setBillboardOrigin(Ogre::v1::BBO_BOTTOM_RIGHT);
		} else {
			LOG_WARNING("Unknow billboard origin mode: " + sValue);
			return billboardSet;
		}
	}
	
	billboardSet->setBillboardsInWorldSpace(
		xmlNode.attribute("inWorldSpace").as_bool(true)
	);
	
	billboardSet->setRenderQueueGroup(
		MGE::RenderQueueGroups::fromString(
			xmlNode.attribute("renderQueueGroups").as_string("GUI_3D_V1"),
			true
		)
	);
	
	billboardSet->setDefaultDimensions(
		xmlTagNode.attribute("width").as_float(),
		xmlTagNode.attribute("height").as_float()
	);
	
	parent.node->attachObject(billboardSet);
	billboardSet->createBillboard(
		MGE::XMLUtils::getValue<Ogre::Vector3>(xmlNode.child("offset"), Ogre::Vector3::ZERO)
	);
	
	billboardSet->beginBillboards(); billboardSet->endBillboards(); // prepare billboard renderable BEFORE set material, otherwise set material will NOT working
	billboardSet->setDatablock(
		Ogre::Root::getSingletonPtr()->getHlmsManager()->getDatablockNoDefault(
			xmlNode.attribute("material").as_string("MAT_MISSING_TEXTURE")
		)
	);
	
	return billboardSet;
}

/**
@page XMLSyntax_SceneConfig

@subsection XMLNode_LookTarget \<lookTarget\>

@c \<lookTarget\> is used to set "lookAt other node or position" and has the following attributes:
  - @c nodeName
    - name of node to get position to lookAt
  - @c relativeTo one of the following string value:
    - @c local
    - @c parent.node
    - @c world
  .
  and following subnodes:
  - @c \<position\>
    - @ref XML_Vector3
    - position to lookAt (used when nodeName is not set or empty
  - @c \<localDirection\>
    - @ref XML_Vector3
    - The local vector considered to be the usual 'direction' of the node (local axis to look at selected point)
    - default NEGATIVE_UNIT_Z
*/
void* MGE::DotSceneLoader::processLookTarget(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context, const MGE::SceneObjectInfo& parent) {
	std::string_view    nodeName      = xmlNode.attribute("nodeName").as_string();
	Ogre::Vector3      position       = MGE::XMLUtils::getValue<Ogre::Vector3>(xmlNode.child("position"), Ogre::Vector3::ZERO);
	Ogre::Vector3      localDirection = MGE::XMLUtils::getValue<Ogre::Vector3>(xmlNode.child("localDirection"), Ogre::Vector3::NEGATIVE_UNIT_Z);
	
	Ogre::Node::TransformSpace relativeTo = Ogre::Node::TS_PARENT;
	std::string_view sValue = xmlNode.attribute("relativeTo").as_string();
	if(sValue == "local")
		relativeTo = Ogre::Node::TS_LOCAL;
	else if(sValue == "parent.node")
		relativeTo = Ogre::Node::TS_PARENT;
	else if(sValue == "world")
		relativeTo = Ogre::Node::TS_WORLD;
	
	try {
		if(!nodeName.empty()) {
			Ogre::SceneNode* pLookNode = MGE::NamedSceneNodes::getSceneNode(nodeName);
			MGE::OgreUtils::updateCachedTransform(pLookNode, false, false, true);
			position = pLookNode->_getDerivedPositionUpdated();
		}
		parent.node->lookAt(position, relativeTo, localDirection);
	} catch(Ogre::Exception &/*e*/) {
		LOG_ERROR("DotSceneLoader: error processing a look target!");
	}
	return nullptr;
}
 
/**
@page XMLSyntax_SceneConfig

@subsection XMLNode_TrackTarget \<trackTarget\>

@c \<trackTarget\> is used for setup auto tracking and has the following attributes:
  - @c nodeName
    - name of node to track
  .
  and following subnodes:
  - @c \<localDirection\>
    - @ref XML_Vector3
    - The local vector considered to be the usual 'direction' of the node (local axis to look at selected point)
    - default NEGATIVE_UNIT_Z
  - @c \<offset\>
    - @ref XML_Vector3
    - If supplied, this is the target point in local space of the target node instead of the origin of the target node
*/
void* MGE::DotSceneLoader::processTrackTarget(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context, const MGE::SceneObjectInfo& parent) {
	std::string_view   nodeName       = xmlNode.attribute("nodeName").as_string();
	Ogre::Vector3      localDirection = MGE::XMLUtils::getValue<Ogre::Vector3>(xmlNode.child("localDirection"), Ogre::Vector3::NEGATIVE_UNIT_Z);
	Ogre::Vector3      offset         = MGE::XMLUtils::getValue<Ogre::Vector3>(xmlNode.child("offset"), Ogre::Vector3::ZERO);
	
	try {
		Ogre::SceneNode* pTrackNode = MGE::NamedSceneNodes::getSceneNode(nodeName);
		parent.node->setAutoTracking(true, pTrackNode, localDirection, offset);
	} catch(Ogre::Exception &/*e*/) {
		LOG_ERROR("DotSceneLoader: error processing a track target!");
	}
	return nullptr;
}

#if 0
void MGE::DotSceneLoader::processPlane(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context, const MGE::SceneObjectInfo& parent) {
	MGE::Utils::String name     = xmlNode.attribute("name").as_string();
	Ogre::Real distance         = xmlNode.attribute("distance").as_float();
	Ogre::Real width            = xmlNode.attribute("width").as_float();
	Ogre::Real height           = xmlNode.attribute("height").as_float();
	
	int xSegments               = xmlNode.attribute("xSegments").as_int(0);
	int ySegments               = xmlNode.attribute("ySegments").as_int(0);
	int numTexCoordSets         = xmlNode.attribute("numTexCoordSets").as_int(0);
	Ogre::Real uTile            = xmlNode.attribute("uTile").as_float();
	Ogre::Real vTile            = xmlNode.attribute("vTile").as_float();
	MGE::Utils::String material = xmlNode.attribute("material").as_string();
	bool hasNormals             = xmlNode.attribute("hasNormals").as_bool(0);
	Ogre::Vector3 normal        = MGE::XMLUtils::getValue<Ogre::Vector3>(xmlNode.child("normal"), Ogre::Vector3::ZERO);
	Ogre::Vector3 up            = MGE::XMLUtils::getValue<Ogre::Vector3>(xmlNode.child("upVector"), Ogre::Vector3::ZERO);
 
	Ogre::Plane plane(normal, distance);
	Ogre::v1::MeshPtr res = Ogre::v1::MeshManager::getSingletonPtr()->createPlane(
		name.getObjStr<Ogre::String>() + "mesh", "General", plane, width, height, xSegments, ySegments, hasNormals,
		numTexCoordSets, uTile, vTile, up
	);
	Ogre::v1::Entity* ent = context->scnMgr->createEntity(name.getObjStr<Ogre::String>(), name.getObjStr<Ogre::String>() + "mesh");
	ent->->setRenderQueueGroup(
		MGE::RenderQueueGroups::fromString(
			xmlNode.attribute("renderQueueGroups").as_string("DEFAULT"),
			true
		)
	);
	ent->setMaterialName(material.getObjStr<Ogre::String>());
	parent.node->attachObject(ent);
}
#endif
