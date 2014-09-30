#!/bin/sh
clear

echo "Setting up development tree in $HOME"
rm -rf $HOME/rpmbuild
rpmdev-setuptree
if [ $? -ne 0 ]; then
	echo "Install the rpm development tools first!"
	exit -1
fi

REDHAT_RPM_CONFIG=`yum list installed | grep "redhat-rpm-config" | wc -c`
if [ $REDHAT_RPM_CONFIG -eq 0 ]; then
        echo "Install redhat-rpm-config first!"
        exit -1
fi;

IS_CENTOS_6_X=`cat /etc/*-release | grep -i "centos release 6."| wc -c`
if [ $IS_CENTOS_6_X -gt 0 ]; then
	CENTOS_DEVTOOLS_1_1=`yum list installed | grep "devtoolset-1.1" | wc -c`
	if [ $CENTOS_DEVTOOLS_1_1 -eq 0 ]; then
	        echo "Install devtools 1.1 first. CentOS 6.X needs it to compile C++ V11 code."
	        echo "Execute: 'cd /etc/yum.repos.d;wget http://people.centos.org/tru/devtools-1.1/devtools-1.1.repo';"
	        echo "Then run: 'yum install devtoolset-1.1-gcc devtoolset-1.1-gcc-c++'"
	        exit -1
	fi;
	scl enable devtoolset-1.1 bash
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