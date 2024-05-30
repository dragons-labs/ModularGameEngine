Scene Files XML syntax                        {#XMLSyntax_SceneConfig}
======================

@tableofcontents

@section SceneConfigFiles .scene xml files

`.scene.xml` files are used to setup scene elements. Root xml element for .scene.xml files is `<scene>`.

For `.scene.xml` loaded by @ref XMLNode_SceneFile in @ref MapConfig direct children of root element (top level elements) are processing
by @ref MGE::LoadAndSave::System with function registred in @ref MGE::LoadAndSave::System::sceneConfigParseListeners "sceneConfigParseListeners"
(full list of supported XML subnodes see @ref XMLSyntax_MapAndSceneConfig) and following additional xml subnodes.


@subsection XMLNode_SceneManager \<sceneManager\>

One of the children of `<scene>` in **first** `.scene.xml` loaded by @ref XMLNode_SceneFile must be `<sceneManager>` element.
It's used for configure SceneManager. See @ref XMLNode_SceneManagerSyntax for details.
This node will be silently ignored in any other (next or sub scene) `.scene.xml` file.


@subsection XMLNode_Resources \<resources\>

One of the children of `<scene>` in **first** `.scene.xml` loaded by @ref XMLNode_SceneFile must be `<resources>` element.

This node is used to define resources groups used by this scene file (and all included sub-scene files).
It use standard @ref XMLNode_ResourcesConfig node syntax.
This node will be silently ignored in any other (next or sub scene) `.scene.xml` file.


@subsection XMLNode_Nodes \<nodes\>

One of the children of `<scene>` in every `.scene.xml` should be `<nodes>` element.

It is internal handled by @ref MGE::LoadAndSave::System::loadDotSceneFile and processing for .scene.xml loaded by @ref XMLNode_SceneFile in @ref MapConfig and for .scene.xml loaded by @ref XMLNode_SubSceneFile in @ref SceneConfigFiles.
For processing this node are used functions registered in @ref MGE::LoadAndSave::System::sceneNodesCreateListeners "sceneNodesCreateListeners"
(full list of supported XML subnodes of `<nodes>` see @ref SceneConfigElementsSyntax "bellow").


@subsection SceneConfigFilesSeeAlso  See also:
* @ref MGE::LoadAndSave::System
* @ref MGE::DataSets::ActorFactory

<p>&nbsp;</p>


@section SceneConfigElementsSyntax .scene \<nodes\> sub-elements XML syntax

This page describes xml syntax for elements registered in @ref MGE::LoadAndSave::System::sceneNodesCreateListeners "sceneNodesCreateListeners" and used to parsing `<nodes>` sub-elements.
