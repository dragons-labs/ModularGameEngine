Game Modding                        {#GameModding}
============

# Mod resources setting

## `conf/resources-MODENAME.xml`

Define paths to mod resources (relative to game main dir, as in default `resources.xml`).

This file is used for:
  - add new map by adding mods dir to MapsMainConfigs group
  - modyfiy or add standard game objects (e.g. actors) by adding own dirs to standard resources groups

## Adding new map

If mod add some new map should have `<Group name="MGE_MapsMainConfigs"><Entry type="dir" path="PATH_TO_MOD_MAPS_CONFIG" /></Group>` in `resources-MODENAME.xml`.
Group name for main maps config (default `MGE_MapsMainConfigs`) is set in `MGEConfig.xml` by `<MapsConfigGroupName/>`, mods must use correct (corresponding to `MGEConfig.xml`) group name.
This group should contains only main map/missions configs (not scene or state files).

Mods can use any name for map config file. It's not possible replace original map by mod – mod can only add new maps.

### New map resources

New maps can use standard resources by including in it config file:

    <Resources>
        <ResourcesConfigFile>resources/Maps/ExtraConfig/standard_maps_resources.xml</ResourcesConfigFile>
    </Resources>

New resources for new map can be add to standard resource group or add to mod own groups.
Using standard group name results adding this resource for all maps, using own names limits access to this resource for this mod only.


## Modyfiy standard game

Mod can add, remove or modify (replace) standard game object (like actors). To do this should add own directories into standard resources groups in `conf/resources-MODENAME.xml`.
Entry for those group in `conf/resources-MODENAME.xml` should be add with `doInit="false"` attribute (will be initialised on map load).

Mods can define resources with this same filename and groupname (as standard resources) for override:
  - actor prototypes
  - actions config files (@ref XMLNode_Actions in @ref MapConfig)
  - world map files      (@ref XMLNode_WorldMap in @ref MapConfig)
Overriding files should have higher priority (set in root xml node attribute) than standard file.

Mods can't define resources with this same filename and groupname (as other resources) for:
  - scene xml files  (@ref XMLNode_SceneFile in @ref MapConfig and @ref XMLNode_SubSceneFile in @ref SceneConfigFiles)
  - state xml files  (@ref XMLNode_StateFile in @ref MapConfig)
  - python scripts   (`<File>` subnodes in @ref XMLNode_Scripts in @ref MapConfig)
  - road graph files (`roadsFileName` + `roadsGroup` in @ref XMLSyntax_WorldMapConfig_RootNode in @ref XMLSyntax_WorldMapConfig)

### ActorsConfig

Mod by putting file with the same name as default file in the same resource group can override actor configs from original file.
Overwriting is on `<ActorPrototype>` level (mod does not have to redefine all prototypes from original file, but when overwrite some prototype must overwrite it completely (full content of `<ActorPrototype>` node).
To override prototypes, in mod file must be set attribute `priority` with value grater then zero in `<Prototypes>` root node (priority is set per file, not per prototype).

### MapsMainConfigs_WorldMaps

Mod by putting file with the same name as default file in the same resource group can override world map.
Overwriting is on `<worldMap>` level (mod must overwrite world map completely (full content of `<worldMap>` root node).
`<worldMap>` in mod file version must set attribute `priority` with value grater then zero.


# Using editor

To use editor for map with own `<Resources>` settings in @ref MapConfig you must set this resources in editor config file (see `<LoadAndSave>` → `<EditorPsedoMapConfigFile>` in @ref XMLSyntax_MainConfig "main config file")

# Sample

See sample mod in `conf/resources-DemoMod.xml` and `resources/DemoMod`.
