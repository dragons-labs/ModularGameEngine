cd cegui-0.8.3

export PKG_CONFIG_PATH=/usr/bin/i686-w64-mingw32-pkg-config
export CC=/usr/bin/i686-w64-mingw32-gcc
export CXX=/usr/bin/i686-w64-mingw32-g++
export RC=/usr/bin/i686-w64-mingw32-windres
export AR=/usr/bin/i686-w64-mingw32-ar
export LD=/usr/bin/i686-w64-mingw32-ld
export RANLIB=/usr/bin/i686-w64-mingw32-ranlib
export STRIP=/usr/bin/i686-w64-mingw32-strip

rm -f *Cache*
cmake . \
	-DCMAKE_SYSTEM_NAME=Windows \
	-DCMAKE_SYSTEM_VERSION=1 \
	-DCMAKE_BUILD_TYPE=Release \
	-DCMAKE_C_COMPILER=$CC \
	-DCMAKE_CXX_COMPILER=$CXX \
	-DCMAKE_CXX_FLAGS="-w -I/01-Game/deps4mingw/include -I/01-Game/deps4mingw/include/python2.7 -I/usr/x86_64-w64-mingw32/include -I/usr/include/AL -I/usr/include -DFREEIMAGE_LIB" \
	-DCMAKE_AR=$AR \
	-DCMAKE_LINKER=$LD \
	-DCMAKE_RANLIB=$RANLIB \
	-DCMAKE_STRIP=$STRIP \
	-DCMAKE_LIBRARY_PATH="/01-Game/deps4mingw/lib/" \
	-DCMAKE_FIND_ROOT_PATH="/usr/i686-w64-mingw32/ /01-Game/deps4mingw/" \
	-DCMAKE_INSTALL_PREFIX=./sdk \
	-DCEGUI_BUILD_PYTHON_MODULES=OFF \
	-DCEGUI_SAMPLES_ENABLED=OFF \
	-DPCRE_LIB=/01-Game/deps4mingw/lib/libpcre-1.dll \
	-DFREEIMAGE_LIB=/01-Game/deps4mingw/lib/libFreeImage.dll \
	-DCMAKE_CXX_STANDARD_LIBRARIES="-lkernel32 -luser32 -lgdi32 -lwinspool -lshell32 -lole32 -loleaut32 -luuid -lcomdlg32 -ladvapi32  -L /01-Game/deps4mingw/lib/ -l:libws2_32.a"

make
