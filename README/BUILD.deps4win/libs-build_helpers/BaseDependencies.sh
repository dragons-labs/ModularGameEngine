#
# Get Python
#

cd $WinDepsDir
wget https://www.python.org/ftp/python/3.7.3/python-3.7.3-amd64.exe
sudo apt-get install wine wine64
sudo dpkg --add-architecture i386 && sudo apt-get update && sudo apt-get install wine32
sudo wine python-3.7.3-amd64.exe
	# manual installation in $WinDepsDir/Python
cp  Python/python3*.dll  $WinLibsDir;  cp  Python/include/*.h  $WinInclDir


#
# Get PCRE
#

cd $WinDepsDir
wget https://datapacket.dl.sourceforge.net/project/gnuwin32/pcre/7.0/pcre-7.0-lib.zip
wget https://netcologne.dl.sourceforge.net/project/gnuwin32/pcre/7.0/pcre-7.0-bin.zip
mkdir pcre; cd pcre; 7z x ../pcre-7.0-lib.zip; 7z x ../pcre-7.0-bin.zip
cp  bin/*.dll lib/*.a  $WinLibsDir;  cp  include/*  $WinInclDir


#
# Get freeimage
#

cd $WinDepsDir
wget https://netix.dl.sourceforge.net/project/freeimage/Binary%20Distribution/3.18.0/FreeImage3180Win32Win64.zip
7z x FreeImage3180Win32Win64.zip
cp  FreeImage/Dist/x64/*.dll  $WinLibsDir;  cp  FreeImage/Dist/x64/*.h  $WinInclDir


#
# Get FreeType
#

cd $WinDepsDir
wget http://repo.msys2.org/mingw/x86_64/mingw-w64-x86_64-freetype-2.10.1-1-any.pkg.tar.xz
tar -xJf mingw-w64-x86_64-freetype-2.10.1-1-any.pkg.tar.xz && mv mingw64 freetype-2.10.1-1
mkdir freetype; cd freetype; 7z x ../freetype-2.3.5-1-bin.zip
cp  freetype-2.10.1-1/bin/*.dll freetype-2.10.1-1/lib/*.a  $WinLibsDir;  cp -r  freetype-2.10.1-1/include/*  $WinInclDir


#
# Build libexpat
#

cd $WinDepsDir
git clone https://github.com/libexpat/libexpat.git
cd libexpat/expat

./buildconf.sh
./configure --host=x86_64-w64-mingw32
make -j5
cp  lib/.libs/*.dll lib/.libs/*.a  $WinLibsDir;  cp  lib/expat*.h  $WinInclDir


#
# Build Boost
#

cd $WinDepsDir
wget https://dl.bintray.com/boostorg/release/1.67.0/source/boost_1_67_0.7z
7z x boost_1_67_0.7z; cd boost_1_67_0

./bootstrap.sh --with-toolset=gcc --with-libraries=system,python,filesystem,serialization,thread,program_options,iostreams,chrono,date_time,atomic,regex
echo "using gcc : mingw64 : x86_64-w64-mingw32-g++ -I/usr/x86_64-w64-mingw32/include -I $WinInclDir -w : <target-os>windows ;" > user-config.jam
echo "using python : 3.7 : : $WinInclDir : $WinLibsDir : <target-os>windows ;" >> user-config.jam
grep 'result.*W32_GETREG' /05-Game-Dependencies-mingw/boost_1_67_0/tools/build/src/tools/python.jam >&/dev/null &&
	sed -i /05-Game-Dependencies-mingw/boost_1_67_0/tools/build/src/tools/python.jam -e '250d'                       # <<< dirty hack
./b2 -d+2 -q  --toolset=gcc-mingw64 --user-config=user-config.jam  --libdir=$WinLibsDir --includedir=$WinInclDir install \
	target-os=windows variant=release threading=multi threadapi=win32 address-model=64 architecture=x86 link=shared,static

# boost 1.54
# ==========
# 
# ln -s /usr/i686-w64-mingw32/lib/librtm.a /usr/i686-w64-mingw32/lib/librt.a
# 
# ( cd tools/build/v2/engine/ && bash build.sh gcc )
# echo 'using gcc : win : i686-w64-mingw32-g++ -I/01-Game/deps4mingw/include -I/01-Game/deps4mingw/include/python2.7 -I/usr/x86_64-w64-mingw32/include -I /usr/include/python2.7 -w : <target-os>windows ;' >> tools/build/v2/user-config.jam
# echo 'using python : 2.7 : [cmd-or-prefix] : [includes] : /01-Game/deps4mingw/lib/ : <target-os>windows ;' >> tools/build/v2/user-config.jam
# 
# PATH="$PATH:/01-Game/deps4mingw-build/boost_1_54_0/tools/build/v2/engine/bin.linuxx86_64"
# bjam --toolset=gcc-win --with-thread --with-system --with-filesystem --with-python --with-serialization --with-chrono --with-date_time --with-program_options link=shared,static threadapi=win32 stage
