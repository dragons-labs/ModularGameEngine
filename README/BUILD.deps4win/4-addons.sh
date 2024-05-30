#
# include enviroment serttings
#

source 0-Enviroment.sh

#
# Build OgreOggSound & OgreVideo
#

cd $WinDepsDir; mkdir ogre-audiovideo; cd ogre-audiovideo
CXX_EXTRA_FLAGS="-DHAVE_EFX=0" CXX_EXTRA_LIBRS="-lOgreHlmsUnlit" runCmake $LinDepsDir/ogre-audiovideo -DCMAKE_MODULE_PATH=$WinLibsDir/cmake/OGRE \
	-DOPENAL_INCLUDE_DIRS=$WinInclDir/AL -DCMAKE_INSTALL_PREFIX=$WinInstDir -DOGRE_PREFIX_DIR=$WinInstDir
make -j5 install


#
# Build BtOgre
#

cd $WinDepsDir; mkdir BtOgre2; cd BtOgre2
CXX_EXTRA_FLAGS="" CXX_EXTRA_LIBRS="" runCmake $LinDepsDir/BtOgre2 -DCMAKE_INSTALL_PREFIX=$WinInstDir
make -j5 install


#
# Build ChromiumEmbedded
#

cd $WinDepsDir

# download package (cef_binary_*_windows64_minimal.tar.bz2) from http://opensource.spotify.com/cefbuilds/index.html and extract to cef_binary
VER="76.1.9+g2cf916e+chromium-76.0.3809.87"
wget "http://opensource.spotify.com/cefbuilds/cef_binary_${VER//+/%2B}_windows64_minimal.tar.bz2"
tar -xjf "cef_binary_${VER}_windows64_minimal.tar.bz2"
mv "cef_binary_${VER}_windows64_minimal" cef_binary

cd cef_binary

tr -d '\r' < cmake/cef_variables.cmake >  cmake/cef_variables-x.cmake
mv cmake/cef_variables-x.cmake cmake/cef_variables.cmake
cat << EOF | patch -p1
diff -u 
--- a/cmake/cef_variables.cmake	2019-08-08 07:01:25.000000000 +0000
+++ b/cmake/cef_variables.cmake	2019-08-08 19:07:42.069041470 +0000
@@ -362,7 +362,7 @@
 # Windows configuration.
 #
 
-if(OS_WINDOWS)
+if(False)
   if (GEN_NINJA)
     # When using the Ninja generator clear the CMake defaults to avoid excessive
     # console warnings (see issue #2120).
EOF

mkdir build; cd build

CXX_EXTRA_FLAGS="-fpermissive -I.." CXX_EXTRA_LIBRS="" runCmake ..
cp /04-Game-Dependencies/cef_binary/include/base/internal/cef_atomicops_x86_gcc.h ../include/base/internal/
make -j5

cp libcef_dll_wrapper/libcef_dll_wrapper.a ../Release/libcef.lib $WinLibsDir
cp -r ../include/ $WinInclDir/cef
ln -s $WinInclDir/cef $WinInclDir/cef/include
