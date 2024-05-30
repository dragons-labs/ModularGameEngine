#!/bin/bash

if [ $# -ne 2 ]; then
	echo "USAGE $0 SOURCE_DIR BUILD_DIR"
	exit
fi

SCRIPTS_DIR=$(dirname $(realpath $0))
SOURCE_DIR=$1
BUILD_DIR=$2

# use pdoc when avalible in version > 0.5.1 (due to https://github.com/pdoc3/pdoc/issues/18)

if python3 -c 'import pdoc; v=pdoc.__version__; print(" found pdoc in version " + v); v=list(map(int,v.split("."))); v[0] > 0 or v[1]> 5 or (v[1]==5 and v[2]>1) or exit(2)' 2> /dev/null; then
	echo " using pdoc"
	cp "${SOURCE_DIR}/buildSystem/scripts/writePyDoc-pdoc.py" "${BUILD_DIR}/writePyDoc.py";
else
	echo " using pydoc"
	cp "${SOURCE_DIR}/buildSystem/scripts/writePyDoc-pydoc.py" "${BUILD_DIR}/writePyDoc.py";
fi
