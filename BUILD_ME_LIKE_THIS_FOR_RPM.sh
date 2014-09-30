#!/bin/sh
clear

echo "Setting up development tree in $HOME"
rm -rf $HOME/rpmbuild
rpmdev-setuptree
if [ $? -ne 0 ]; then
	echo "Install the rpm development tools first!"
	exit -1
fi

REDHAT_RPM_CONFIG=`yum list installed | grep "redhat-rpm-config" | wc -l`
if [ $REDHAT_RPM_CONFIG -eq 0 ]; then
        echo "Install redhat-rpm-config first!"
        exit -1
fi;

export TISSUESTACK_BUILD_VERSION=1.5

CURRENT_DIR=`pwd`

echo "Calling TissueStack make with target dist"
cd $CURRENT_DIR/src/c++;make dist

echo "Copying spec file and source tar"
cp -f $CURRENT_DIR/rpm/tissuestack.spec $HOME/rpmbuild/SPECS 
#cp -f /tmp/tissuestack_build/tissuestack-$TISSUESTACK_BUILD_VERSION.tar.gz $HOME/rpmbuild/SOURCES
cd $HOME/rpmbuild/SPECS/

echo "Calling RPM build now ..."
rpmbuild -bb tissuestack.spec