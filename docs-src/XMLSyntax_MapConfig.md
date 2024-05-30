Map Config XML syntax                        {#XMLSyntax_MapConfig}
=====================

@tableofcontents

@section MapConfig Map Config

Map (aka mission) config file is used to configure game engine elements and setup scene for selected map/mission.

To be listed in "start game" menu files should be added to resource group specified by `<MapsConfigGroupName>` tag in @ref XMLSyntax_MainConfig "main config file".

Root xml element is `<Mission>`. It contains described @ref MapConfigSubnodes "below" xml subnodes and:
  - `<Name>` subnode used for getting name to show in "start game" menu and used to generate suggested save file name.
  - `<Description>` subnode used for getting description to show in "start game"
  - `<LoadScreen>` subnode with attributes @c file and @c group for set mission loading screen image
    and with optional set of @ref XMLNode_ResourcesConfigEntry nodes (as in `<Group>` node from @ref XMLNode_ResourcesConfig) for define resources group (with name setting by @c group attribute) needed by LoadScreen

*Order of XML tags is importatnt*, e.g. before restoring of state some object they must be created.

@subsection MapConfigSeeAlso See also:
* @ref MGE::LoadAndSave::System
* @ref XMLSyntax_MapAndSceneConfig

@section MapConfigSubnodes Mission Configuration XML Nodes

This page describes xml syntax for elements that can be used only in @ref MapConfig.
For elements registered in @ref MGE::LoadAndSave::System::sceneConfigParseListeners "sceneConfigParseListeners" (that can be used in @ref MapConfig and @ref SceneConfigFiles) see @ref XMLSyntax_MapAndSceneConfig.
