#!/bin/bash

if [ $# -ne 1 ]; then
	echo "USAGE $0 SOURCE_DIR"
	exit
fi

cd $1

GITSTATUS=`git status --porcelain`
if [ "$GITSTATUS" != "" ]; then
	GITSTATUS=" ++"
fi

echo "const char* ENGINE_GIT_VERSION=\"`git rev-parse HEAD`$GITSTATUS\";"
echo "const char* ENGINE_BUILD_TIME=\"`date +"%Y-%m-%d %H:%M:%S %Z (unix=%s)"`\";"
