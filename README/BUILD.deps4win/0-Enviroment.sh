LinDepsDir="/04-Game-Dependencies"
WinDepsDir="/04-Game-Dependencies-4-Windows"
WinInstDir="$WinDepsDir/mingw64"
WinLibsDir="$WinInstDir/lib"
WinDllsDir="$WinInstDir/bin"
WinInclDir="$WinInstDir/include"

export PKG_CONFIG_PATH=/usr/bin/x86_64-w64-mingw32-pkg-config
export CC=/usr/bin/x86_64-w64-mingw32-gcc
export CXX=/usr/bin/x86_64-w64-mingw32-g++
export RC=/usr/bin/x86_64-w64-mingw32-windres
export AR=/usr/bin/x86_64-w64-mingw32-ar
export LD=/usr/bin/x86_64-w64-mingw32-ld
export RANLIB=/usr/bin/x86_64-w64-mingw32-ranlib
export STRIP=/usr/bin/x86_64-w64-mingw32-strip

runCmake() {
	srcDir=$1; shift
	cmake $srcDir \
	-DCMAKE_SYSTEM_NAME=Windows -DCMAKE_SYSTEM_VERSION=1 \
	-DCMAKE_C_COMPILER=$CC -DCMAKE_CXX_COMPILER=$CXX -DCMAKE_AR=$AR -DCMAKE_LINKER=$LD -DCMAKE_RANLIB=$RANLIB -DCMAKE_STRIP=$STRIP \
	-DCMAKE_INCLUDE_PATH="$WinInclDir;/usr/x86_64-w64-mingw32/include/" \
	-DCMAKE_LIBRARY_PATH="$WinLibsDir;/usr/x86_64-w64-mingw32/lib/" \
	-DCMAKE_FIND_ROOT_PATH="$WinDepsDir;/usr/x86_64-w64-mingw32" -DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY -DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY \
	-DCMAKE_CXX_FLAGS="$CXX_EXTRA_FLAGS -w -I/usr/x86_64-w64-mingw32/include -I$WinInclDir" \
	-DCMAKE_CXX_STANDARD_LIBRARIES="-lkernel32 -luser32 -lgdi32 -lwinspool -lshell32 -lole32 -loleaut32 -luuid -lcomdlg32 -ladvapi32  -L $WinLibsDir -l:libws2_32.a $CXX_EXTRA_LIBRS" \
	-DCMAKE_BUILD_TYPE=Release "$@"
}
