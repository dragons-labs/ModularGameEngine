Prototypes XML syntax                        {#XMLSyntax_Prototypes}
=====================

@tableofcontents

@section XMLSyntax_PrototypeConfigFile Prototypes config file

Prototypes is stored in XML files and can be referenced with @ref XMLNode_PrototypeRef.
Root node in Prototypes config file must be `<Prototypes>`.
This node can have optional `priority` argument, it value is used to select between files with this same name and this same resource group (default 0, used is file with highest value).
This node contain set of `<ActorPrototype>` nodes.

See too: @ref MGE::DataSets::BasePrototype::getPrototypeXML


@subsection XMLSyntax_ActorPrototype \<ActorPrototype\>

`<ActorPrototype>` is used for describing single prototype and have next attributes:
	- `name` with name of this prototype (unique in single file)
	.
	and next subnodes (for creating prototype itself, see @ref MGE::DataSets::BasePrototypeImpl::restore):
	- @ref #XMLNode_Property, can be used multiple times
	- @ref #XMLNode_Component with components to set for this prototype, can be used multiple times
	  (describe the prototype itself)
	.
	and next subnodes (for actors based on this prototype, see @ref MGE::DataSets::ActorFactory::_createActor)
	- `<ActorComponents>` @ref equivalent of #XMLNode_Component used for components to set for actors created on this prototype
	  (describe the actors created with this prototype)
	- any sensible (intended for use as child of @ref XMLNode_Node) node registered for processing .scene file (see @ref SceneConfigElementsSyntax), typically @ref XMLNode_Item or @ref XMLNode_Entity
	- `<scale>` @ref XML_Vector3 default scale for main scene node of created 3D objects

@note
	Some components should be in both `<Components>` and `<ActorComponents>` nodes.
