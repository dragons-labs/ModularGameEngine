cd ogre-1.9

export PKG_CONFIG_PATH=/usr/bin/i686-w64-mingw32-pkg-config
export CC=/usr/bin/i686-w64-mingw32-gcc
export CXX=/usr/bin/i686-w64-mingw32-g++
export RC=/usr/bin/i686-w64-mingw32-windres
export AR=/usr/bin/i686-w64-mingw32-ar
export LD=/usr/bin/i686-w64-mingw32-ld
export RANLIB=/usr/bin/i686-w64-mingw32-ranlib
export STRIP=/usr/bin/i686-w64-mingw32-strip

#
# należy wykomentować dodawanie "-DBOOST_THREAD_VERSION=4" w CMake/Dependencies.cmake
#
# w przeciwnym razie nalezy ta flage dodawac do plikow budowach z naglowkami OGRE gdyz zmienia ona rozmiar
# (pokazywany w pliku uzyskiwanym poprzez opcję -fdump-class-hierarchy-all) i offsety pomiędzy składowymi
# obiektow klasy Ogre::Terrain pomiedzy wersja dll komilowana z ta flaga a binarkami budowanymi bez niej
#

rm -f *Cache*
cmake . \
	-DCMAKE_SYSTEM_NAME=Windows \
	-DCMAKE_SYSTEM_VERSION=1 \
	-DCMAKE_BUILD_TYPE=Release \
	-DCMAKE_C_COMPILER=$CC \
	-DCMAKE_C_FLAGS="-w -I/01-Game/deps4mingw/include -I/01-Game/deps4mingw/include/python2.7 -I/usr/x86_64-w64-mingw32/include -I/usr/include/AL -I/usr/include" \
	-DCMAKE_CXX_COMPILER=$CXX \
	-DCMAKE_CXX_FLAGS="-w -I/01-Game/deps4mingw/include -I/01-Game/deps4mingw/include/python2.7 -I/usr/x86_64-w64-mingw32/include -I/usr/include/AL -I/usr/include" \
	-DCMAKE_AR=$AR \
	-DCMAKE_LINKER=$LD \
	-DCMAKE_RANLIB=$RANLIB \
	-DCMAKE_STRIP=$STRIP \
	-DCMAKE_LIBRARY_PATH="/01-Game/deps4mingw/lib/" \
	-DCMAKE_FIND_ROOT_PATH="/usr/i686-w64-mingw32 /01-Game/deps4mingw/" \
	-DCMAKE_INSTALL_PREFIX=./sdk \
	-DOGREDEPS_SOURCE_DIR=./ogredeps \
	-DOGRE_BUILD_DOCS=OFF \
	-DOGREDEPS_BUILD_OIS=OFF \
	-DCMAKE_CXX_STANDARD_LIBRARIES="-lkernel32 -luser32 -lgdi32 -lwinspool -lshell32 -lole32 -loleaut32 -luuid -lcomdlg32 -ladvapi32  -L /01-Game/deps4mingw/lib/ -lboost_system -lboost_thread -lboost_chrono" \

make
