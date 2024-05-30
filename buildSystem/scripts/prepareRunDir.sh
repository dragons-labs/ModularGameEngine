#!/bin/bash

if [ $# -ne 2 ]; then
	echo "USAGE $0 SOURCE_DIR BUILD_DIR"
	exit
fi

echo "prepare run enviroment ..."

SCRIPTS_DIR=$(dirname $(realpath $0))
SOURCE_DIR=$1
BUILD_DIR=$2


# prepare needed dirs

mkdir -p $BUILD_DIR/conf $BUILD_DIR/Fonts $BUILD_DIR/GPUCache $BUILD_DIR/hlmsDebug $BUILD_DIR/saves/autosave


# copy config files

rm $BUILD_DIR/conf/* 2>/dev/null
for f in $SOURCE_DIR/resources-src/ConfigFiles/*; do
	if [ "${f%.in}" = "$f" ]; then
		ln -sfr "$f" $BUILD_DIR/conf/
	fi
done


# download DejaVu fonts

NEED_FONTS=false
for f in DejaVuSans-Bold.ttf  DejaVuSansMono-Bold.ttf DejaVuSansMono.ttf; do
	[ ! -e $BUILD_DIR/Fonts/dejavu/$f ] && NEED_FONTS=true && break
done

if $NEED_FONTS; then
	wget -O - 'http://sourceforge.net/projects/dejavu/files/dejavu/2.37/dejavu-fonts-ttf-2.37.tar.bz2' | tar -xjf - -C $BUILD_DIR/Fonts/
	mv $BUILD_DIR/Fonts/dejavu-fonts-*/ttf $BUILD_DIR/Fonts/dejavu/
	mv $BUILD_DIR/Fonts/dejavu-fonts-*/{AUTHORS,LICENSE,README.md} $BUILD_DIR/Fonts/dejavu/
	rm -fr $BUILD_DIR/Fonts/dejavu-fonts-*
fi

# linking to resources dirs

mkdir -p $BUILD_DIR/resources
ln -srfn $SOURCE_DIR/resources $BUILD_DIR/resources/General
if [ ! -d $BUILD_DIR/resources/SampleMod ]; then
	( cd "$SOURCE_DIR" && git submodule update --init SampleMod )
	ln -sr $SOURCE_DIR/SampleMod $BUILD_DIR/resources/SampleMod   
fi

echo "prepare run enviroment ... done"
