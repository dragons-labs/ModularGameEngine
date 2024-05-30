Main Config XML syntax                    {#XMLSyntax_MainConfig}
======================

@tableofcontents

@section MainConfigFile Main Config File

Main engine config file is xml file located in path specified by `MGE_MAIN_CONFIG_FILE_DEFAULT_PATH` preprocessor constant (default @c ./conf/MGEConfig.xml).
Location can be changed by `--config-file` command line option.

Root XML element is @c \<MGEConfig\> and contains next sub nodes:
  - @c \<LogSystem\> for log configuration:
    - @c \<LogFile\>
      - path to log file
  - @c \<Autostart\> for configuration modules started with engine
    - see @ref AutostartSyntax for all other nodes
    - order of nodes is important, see sample resources-src/ConfigFiles/MGEConfig.xml.in
  - @c \<LoadAndSave\> for load-save configuration:
    - @c \<MapsConfigGroupName\>
      - resources group name for maps/missions config files
    - @c \<SaveDirectrory\>
      - path to game-saves directory
    - @c \<AutoSaveDirectrory\>
      - path to game-saves directory for autosave (on load, exit, etc)
    - @c \<OnCrashSaveFile\>
      - path to game-save file to write "on crash" save
    - @c \<DefaultSceneFilesDirectory\>
      - path to direcory with .scene file used for game (for open/save dialog default location)
    - @c \<EditorPsedoMapConfigFile\>
      - path to editor config in @ref MapConfig format to load via load-save system

@section AutostartSyntax \<Autostart\> Syntax Description
