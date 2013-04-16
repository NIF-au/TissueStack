#!/bin/sh
clear
echo "Setting up development tree in $HOME"
rm -rf $HOME/rpmbuild
rpmdev-setuptree
if [ $? -ne 0 ]; then
	echo "Install the rpm development tools first!"
	exit -1
fi;
if [ $# -eq 0 ]
	then
	echo "No Version Number supplied. Please call the script like this: BUILD_ME_LIKE_THIS_FOR_RPM.sh 1.1"
fi

make clean
make dist VERSION=$1 DATA_PATH=$2 APPLICATION_PATH=$3

cp -f rpm/SPECS/tissuestack.spec $HOME/rpmbuild/SPECS 
cp -rf /tmp/build/tissuestack_build/tissuestack-$1.tar.gz $HOME/rpmbuild/SOURCES/
cd $HOME/rpmbuild/SPECS/
#rpmbuild -bb tissuestack.spec
