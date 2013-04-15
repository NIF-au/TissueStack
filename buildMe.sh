#!/bin/sh
clear
echo "Setting up development tree in $HOME"
rm -rf $HOME/rpmbuild
rpmdev-setuptree
if [ $? -ne 0 ]; then
	echo "Install the rpm development tools first!"
	exit -1
fi;
cp -f SPECS/minc.spec $HOME/rpmbuild/SPECS 
cp -rf SOURCES/* $HOME/rpmbuild/SOURCES/
cd $HOME/rpmbuild/SPECS/
rpmbuild -bb minc.spec
