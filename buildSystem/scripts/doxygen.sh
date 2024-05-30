#!/bin/bash

if [ $# -ne 3 ]; then
	echo "USAGE $0 SOURCE_DIR BUILD_DIR [DOXYGEN_EXECUTABLE]"
	exit
fi

SCRIPTS_DIR=$(dirname $(realpath $0))
SOURCE_DIR=$1
BUILD_DIR=$2
DOXYGEN_EXECUTABLE=${3-doxygen}

cd ${BUILD_DIR}

mkdir -p Documentation/html

if [ -f fastTarget.txt ]; then
	echo "fastTarget: skip run Doxygen"
else
	echo "<html> <head> <title></title> <meta http-equiv=\"refresh\" content=\"0; url=html/index.html\"> </head> <body>" > Documentation/index.html
	echo "<p><a href=\"html/index.html\">INDEX</a></p>" >> Documentation/index.html
	echo "</body> </html>" >> Documentation/index.html
	
	awk 'BEGIN {p=1} /NO_DOXYGEN_START/ {p=0} /NO_DOXYGEN_LINE/ {next} p==1 {print $0} /NO_DOXYGEN_END/ {p=1}' < ${SOURCE_DIR}/README/README.html > README.html_incl
	ln -frs ${SOURCE_DIR}/README/*.svg ${SOURCE_DIR}/README/COPYRIGHT.code ${SOURCE_DIR}/README/BUILD.deps ${SOURCE_DIR}/README/COPYRIGHT.libs Documentation/html/
	
	${DOXYGEN_EXECUTABLE} Doxyfile >Documentation.log 2>&1
	
	# replace '#include <link to header source>' by '#include <link to header documentation>'
	( cd Documentation/html
		for f in class*.html struct**.html; do
			sed -e 's@#include &lt;<a class="el" href="\([^"]*\)_source.html">@#include \&lt;<a class="el" href="\1.html">@g' -i "$f"
		done;
	)
	
	# fix navtree - remove duplicated entry
	( cd Documentation/html
		
		# get numeric index of modules subtree
		modules_idx=`grep -h 'modules.html' navtreeindex* | awk -F'[][,]' '{print $2}'`
		
		# get all members of modules subtree
		grep -h '":\['$modules_idx',' navtreeindex* | awk -F'"' '{print $2}' > allModulesMembers.MGE.txt
		
		# find duplications in other namespaces
		grep -h -f allModulesMembers.MGE.txt navtreeindex* | grep -v '":\[3,' > duplicated.MGE.txt
		
		# and remove them
		for f in navtreeindex*; do
			grep -vFf duplicated.MGE.txt $f > navtree__TMP__index.js
			mv navtree__TMP__index.js $f
		done
		
		rm allModulesMembers.MGE.txt duplicated.MGE.txt
	)
fi
