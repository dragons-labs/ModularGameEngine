#!/bin/sh

# export path to "lib" subdir in executable dir
export LD_LIBRARY_PATH=lib
export PYTHONPATH=lib/python
export CEGUI_MODULE_DIR=lib/cegui-9999.0

# fix some config files
sed -e 's#^Plugin=/usr/local/lib/#Plugin=lib/#' -i conf/plugins.cfg

# execute game
exec ./Game
