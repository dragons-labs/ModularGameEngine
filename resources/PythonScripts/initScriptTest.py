#
# demo script for use with `--exec-script` command line option
#

import sys, os
baseDir = os.getcwd()
def loadScriptFile(path):
	exec(open(baseDir + "/" + path).read())

escMenu = MGE.GUIEscMenu.get()
escMenu.show()

loadScriptFile("resources/General/PythonScripts/engine_debug.py")

console = MGE.GUIConsole.get()
console.show()
