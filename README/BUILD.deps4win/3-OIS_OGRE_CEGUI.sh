#
# include enviroment serttings
#

source 0-Enviroment.sh

#
# Build OIS
#

cd $WinDepsDir; mkdir OIS; cd OIS
CXX_EXTRA_FLAGS="" CXX_EXTRA_LIBRS="" runCmake $LinDepsDir/OIS -DCMAKE_INSTALL_PREFIX=$WinInstDir
make -j5 install


#
# Build Ogre
#

cd $WinDepsDir; mkdir Ogre; cd Ogre
CXX_EXTRA_FLAGS="" CXX_EXTRA_LIBRS="" runCmake $LinDepsDir/ogre -DCMAKE_INSTALL_PREFIX=./sdk \
	-DOGRE_BUILD_TOOLS=0 -DOGRE_BUILD_SAMPLES=0 -DOGRE_BUILD_SAMPLES2=0 -DOGRE_BUILD_DOCS=0 -DRapidjson_INCLUDE_DIR=$WinInclDir/rapidjson
sed -i CMakeCache.txt -e 's@OIS_INCLUDE_DIR:PATH=.*$@OIS_INCLUDE_DIR:PATH='$WinInclDir'/ois@'  # <<< dirty hack
cmake .
make -j5 install
cp sdk/bin/Release/*.dll $WinDllsDir;  cp sdk/lib/Release/*.a sdk/lib/Release/opt/*.a $WinLibsDir;  cp -r sdk/include/* $WinInclDir; cp -r sdk/CMake $WinLibsDir/cmake/OGRE


#
# Build CEGUI
#

cd $WinDepsDir; mkdir cegui; cd cegui
CXX_EXTRA_FLAGS="-I" CXX_EXTRA_LIBRS="" runCmake $LinDepsDir/cegui -DCMAKE_INSTALL_PREFIX=$WinInstDir \
	-DCEGUI_STRING_CLASS=1 -DCEGUI_BUILD_PYTHON_MODULES_SWIG=ON -DCEGUI_BUILD_RENDERER_OGRE=ON -DCEGUI_SAMPLES_ENABLED=OFF -D CEGUI_BUILD_APPLICATION_TEMPLATES=OFF -D CEGUI_BUILD_PYTHON_MODULES_PYPLUSPLUS=OFF -DCEGUI_BUILD_XMLPARSER_TINYXML=OFF -DCEGUI_BUILD_XMLPARSER_RAPIDXML=OFF
sed -i CMakeCache.txt -e 's@/usr/local/include/OGRE@/06-Game-Dependencies-mingw/mingw64/include/OGRE@g' # <<< dirty hack
cmake .
make -j5 install
