#
# get stuff from msys2
#

PKGS="
	expat-2.2.6-1-any
	pcre-8.43-1-any
	
	zlib-1.2.11-7-any
	bzip2-1.0.6-6-any
	xz-5.2.4-1-any
	zziplib-0.13.69-1-any
	
	rapidjson-1.1.0-1-any
	tinyxml-2.6.2-4-any
	glm-0.9.9.4-1-any
	
	boost-1.67.0-2-any
		icu-64.2-1-any
	
	python3-3.7.3-3-any
		libffi-3.2.1-4-any
		mpdecimal-2.4.2-1-any
	
	freeimage-3.18.0-1-any
		libpng-1.6.37-3-any
		libjpeg-turbo-2.0.2-1-any
		openjpeg2-2.3.1-1-any
		libtiff-4.0.10-1-any
			zstd-1.4.0-1-any
		libraw-0.19.2-1-any
			jasper-2.0.16-1-any
			lcms2-2.9-1-any
		jxrlib-1.1-3-any
		libwebp-1.0.2-1-any
		openexr-2.2.1-1-any
			ilmbase-2.2.1-1-any
				libwinpthread-git-7.0.0.5480.e14d23be-1-any
	
	freetype-2.10.0-1-any
		graphite2-1.3.13-1-any
		harfbuzz-2.5.0-1-any
			glib2-2.60.1-1-any
				gettext-0.19.8.1-8-any
					libiconv-1.16-1-any
	
	openal-1.19.1-1-any
	libvorbis-1.3.6-1-any
	libogg-1.3.3-1-any
	libtheora-1.1.1-4-any
	
	bullet-2.87-1-any
		freeglut-3.0.0-4-any
		openvr-1.0.16-1-any
	
	poco-1.9.0-1-any
	
	# we do not need this - we use and copy DLLs from /usr/lib/gcc/x86_64-w64-mingw32/
	# gcc-libs-8.3.0-2-any
	
	# debug only
	gdb-8.3-8-any
		readline-8.0.000-4-any
		termcap-1.3.1-5-any
	ntldd-git-r15.e7622f6-2-any
"

PKGS=`echo "$PKGS" | sed -e 's@#.*$@@'`


mkdir -p msys2-pkgs
cd msys2-pkgs
	for p in $PKGS; do
		[ ! -e mingw-w64-x86_64-$p.pkg.tar.xz ] && wget http://repo.msys2.org/mingw/x86_64/mingw-w64-x86_64-$p.pkg.tar.xz && tar -xJf mingw-w64-x86_64-$p.pkg.tar.xz -C ..
	done;
cd ..

# wsparcie dla std::future w mingw
git clone https://github.com/meganz/mingw-std-threads.git
cp  mingw-std-threads/mingw.*.h  mingw64/include
