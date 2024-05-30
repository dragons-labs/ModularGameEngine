Actor Component XML syntax                        {#XMLSyntax_ActorComponent}
==========================

@tableofcontents

@section ActorComponents Actor Components

Actor components is a way to modular expand Actors. Each component provides a set of functionality (eg. object in Ogre 3D World, owner of other actors objects, ...).

Component are identified by classID (aka primary typeID) and provides set of typeIDs (of interfaces it implements). See also: @ref objectComponents.cpp

@subsection XMLNode_Component \<Component\>

`<Component>` node is used in configuration of prototypes and actors (and in actors saves). This node has the following attributes:
  - `classID`  numeric id (or string converted to numeric id value - see @ref MGE::DataSets::ComponentFactory::nameToID) of component class
  - `typeID`   numeric id (or string converted to numeric id value - see @ref MGE::DataSets::ComponentFactory::nameToID) of interfaces registered in actor provaided by this class (optional)
Content of `<Component>` node is used to configure / restore specific component.

@section XMLSyntax_ActorComponentRestore XML Restore Syntax for Actor Components
