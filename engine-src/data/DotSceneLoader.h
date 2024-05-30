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

namespace MGE { struct LoadingContext; }
namespace MGE { struct Module; }
namespace MGE { struct SceneObjectInfo; }

namespace pugi { class xml_node; }

namespace Ogre {
	class SceneManager;
	class SceneNode;
	class Item;
	class Light;
	class ParticleSystem;
	namespace v1 {
		class Entity;
		class BillboardSet;
	}
}

namespace MGE {

/// @addtogroup LoadingSystem
/// @{
/// @file

/**
 * @brief .scene xml file loader
 */
namespace DotSceneLoader {
	/**
	 * @brief process @ref XMLNode_Environment (can be used also in map config, not only in .scene files)
	 * 
	 * see @ref MGE::ConfigParser::SceneConfigParseFunction for argument description
	 */
	MGE::Module* processEnvironment(const pugi::xml_node& xml, const MGE::LoadingContext* context);
	
	/**
	 * @brief process @ref XMLNode_SubSceneFile
	 * 
	 * see @ref MGE::SceneLoader::parseSceneXMLNode for argument description
	 */
	void* processSceneFile(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context, const MGE::SceneObjectInfo& parent);
	
	/**
	 * @brief process @ref XMLNode_Node
	 * 
	 * see @ref MGE::SceneLoader::parseSceneXMLNode for argument description
	 *
	 * @note if @a LoadingContext::linkToXML (in @a context) is true, add to created Ogre::SceneNode "xml" user bindings with points to XML configuration node.
	 */
	Ogre::SceneNode* processNode(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context, const MGE::SceneObjectInfo& parent);
	
	/**
	 * @brief process @ref XMLNode_Item
	 * 
	 * see @ref MGE::SceneLoader::parseSceneXMLNode for argument description
	 */
	Ogre::Item* processItem(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context, const MGE::SceneObjectInfo& parent);
	
	/**
	 * @brief process @ref XMLNode_Entity
	 * 
	 * see @ref MGE::SceneLoader::parseSceneXMLNode for argument description
	 */
	Ogre::v1::Entity* processEntity(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context, const MGE::SceneObjectInfo& parent);
	
	/**
	 * @brief process @ref XMLNode_Light
	 * 
	 * see @ref MGE::SceneLoader::parseSceneXMLNode for argument description
	 */
	Ogre::Light* processLight(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context, const MGE::SceneObjectInfo& parent);
	
	/**
	 * @brief process @ref XMLSyntax_SceneConfig
	 * 
	 * see @ref MGE::SceneLoader::parseSceneXMLNode for argument description
	 */
	Ogre::ParticleSystem* processParticleSystem(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context, const MGE::SceneObjectInfo& parent);
	
	/**
	 * @brief process @ref XMLNode_BillboardSet
	 * 
	 * see @ref MGE::SceneLoader::parseSceneXMLNode for argument description
	 */
	Ogre::v1::BillboardSet* processBillboardSet(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context, const MGE::SceneObjectInfo& parent);
	
	/**
	 * @brief process @ref XMLNode_LookTarget
	 * 
	 * see @ref MGE::SceneLoader::parseSceneXMLNode for argument description
	 */
	void* processLookTarget(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context, const MGE::SceneObjectInfo& parent);
	
	/**
	 * @brief process @ref XMLNode_TrackTarget
	 * 
	 * see @ref MGE::SceneLoader::parseSceneXMLNode for argument description
	 */
	void* processTrackTarget(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context, const MGE::SceneObjectInfo& parent);
	
	//void processPlane(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context, const MGE::SceneObjectInfo& parent);
};

/// @}

}
