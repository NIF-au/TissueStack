#!/bin/sh
clear
echo "Setting up development tree in $HOME"
rm -rf $HOME/rpmbuild
rpmdev-setuptree
if [ $? -ne 0 ]; then
	echo "Install the rpm development tools first!"
	exit -1
fi;

REDHAT_RPM_CONFIG=`yum list installed | grep "redhat-rpm-config" | wc -l`
if [ $REDHAT_RPM_CONFIG -eq 0 ]; then
	echo "Install redhat-rpm-config first!"
	exit -1
fi;

cp -f SPECS/minc.spec $HOME/rpmbuild/SPECS 
cp -rf SOURCES/* $HOME/rpmbuild/SOURCES/
cd $HOME/rpmbuild/SPECS/

export QA_RPATHS=$[ 0x0002 ]
rpmbuild -bb minc.spec
