#!/bin/bash

if [ $# -ne 2 ]; then
	echo "USAGE $0 SOURCE_DIR BUILD_DIR"
	exit
fi

SOURCE_DIR=$1
BUILD_DIR=$2

extractEnum() {
	# $1 == file with enum definition
	# $2 == name of enum to extract
	sed -e 's#//.*$##g' "$1" | tr -d '\n' | sed -e 's#[[:blank:]]\+# #g' | awk 'BEGIN {RS="enum[\t\n ]*'$2'"; FS="[{}]"} NR==2 { gsub("//.*$", "", $2); print $2 }'| tr -d ' '
}

convertEnumToSedCmd() {
	# $1 == file with enum definition
	# $2 == name of enum to extract
	extractEnum "$1" $2 | awk 'BEGIN {RS="[,\n]"; FS="="} {printf("-e s@%s@%s@g ", $1, $2)}'
}

sed_opt=$( convertEnumToSedCmd "$SOURCE_DIR/engine-src/rendering/utils/RenderQueueGroups.h" RenderQueueGroups )

mkdir -p $BUILD_DIR
rm $BUILD_DIR/*.compositor 2> /dev/null

for infile in $SOURCE_DIR/resources-src/OGRE_compositor/*.compositor; do
	outfile="$BUILD_DIR/`basename $infile`"
	
	echo "// DO NOT EDIT: this file is machine generated" > $outfile
	sed -e'/^\/\/ PREPOCESS:/d' $sed_opt $infile >> $outfile
done;

touch $BUILD_DIR/.compositor.timestamp
