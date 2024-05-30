DSTDIR=/mnt/MGE4win
DEPDIR=/04-Game-Dependencies-4-Windows


mkdir -p $DSTDIR/{bin,conf,saves,lib}

cp     /usr/lib/gcc/x86_64-w64-mingw32/8.3-win32/*.dll    $DSTDIR/bin/
cp     $DEPDIR/mingw64/bin/*.dll                          $DSTDIR/bin/
cp -rL $DEPDIR/mingw64/lib/python3.7                      $DSTDIR/lib/
cp -rL /usr/local/share/OGRE/Media/Hlms/                  $DSTDIR/lib/OGRE_Media_Hlms

cp     $DEPDIR/Ogre/Components/Python/Ogre.py \
       $DEPDIR/Ogre/lib/_Ogre.pyd                         $DSTDIR/lib/python3.7/

cp     $DEPDIR/cegui/cegui/src/ScriptModules/SWIG/CEGUI.py \
       $DEPDIR/cegui/bin/_CEGUI.pyd                       $DSTDIR/lib/python3.7/

cp     $DEPDIR/cef_binary/Resources/{cef*.pak,icudtl.dat} \
       $DEPDIR/cef_binary/Release/*.{dll,bin}             $DSTDIR/bin/


cp -rL /01-Game/resources/                                $DSTDIR/
cp -rL /01-Game/buildDir-win/OgreCompositor/              $DSTDIR/resources/General/
cp     /01-Game/buildDir-win/Game.exe                     $DSTDIR/bin/
cp     /01-Game/buildDir-win/cef_sub_process.exe          $DSTDIR/bin/

cp     /01-Game/SRC/resources-src/ConfigFiles/editor.state.xml   $DSTDIR/conf/
cp     /01-Game/SRC/resources-src/ConfigFiles/ogre.cfg           $DSTDIR/conf/

sed   /01-Game/SRC/resources-src/ConfigFiles/resources.xml \
		-e 's#path="resources#path="../resources#g' \
	> $DSTDIR/conf/resources.xml
sed   /01-Game/SRC/resources-src/ConfigFiles/MGEConfig.xml.in \
		-e 's#@OGRE_HLMS_DIR@#../lib/OGRE_Media_Hlms#' \
		-e 's#dge-game.log#../dge-game.log#' \
		-e 's#resources/#../resources/#g' \
		-e 's#conf/#../conf/#g' \
		-e 's#saves#../saves#g' \
	> $DSTDIR/conf/MGEConfig.xml
sed   /01-Game/SRC/resources-src/ConfigFiles/editor.xml \
		-e 's#conf/#../conf/#g' \
	> $DSTDIR/conf/editor.xml

cat <<EOF > $DSTDIR/conf/plugins.cfg
PluginFolder=
Plugin=RenderSystem_GL3Plus.dll
Plugin=Plugin_ParticleFX.dll
Plugin=Plugin_OggSound.dll
Plugin=Plugin_TheoraVideoSystem.dll
EOF

# debug only
cp $DEPDIR/mingw64/bin/{gdb,ntldd}.exe $DSTDIR/bin/
mkdir -p $DSTDIR/share; cp -r $DEPDIR/mingw64/share/gdb/ $DSTDIR/share
