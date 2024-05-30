#
# Install mingw
#

sudo aptitude install mingw-w64


#
# Prepare build dir
#

source 0-Enviroment.sh

sudo mkdir "$WinDepsDir"
sudo chown `whoami` "$WinDepsDir"
mkdir -p "$WinLibsDir" "$WinInclDir"
