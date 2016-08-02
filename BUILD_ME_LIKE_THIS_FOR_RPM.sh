#!/bin/sh
clear

echo "Setting up development tree in $HOME"
rm -rf $HOME/rpmbuild
rpmdev-setuptree
if [ $? -ne 0 ]; then
	echo "Install the rpm development tools first!"
	exit -1
fi

export TISSUESTACK_BUILD_VERSION=2.2
export IS_RELEASE=1

SPEC_FILE=tissuestack.spec
TARGET=server

IS_SUSE=`cat /etc/*-release | grep -i "suse"| wc -c`
IS_CENTOS=`cat /etc/*-release | grep -i "centos"| wc -c`
IS_CENTOS_6_X=`cat /etc/*-release | grep -i "centos release 6."| wc -c`
PACKAGE_MANAGER="dnf"
if [ $IS_CENTOS_6_X -gt 0 ]; then
    PACKAGE_MANAGER="yum"
fi;

REDHAT_RPM_CONFIG=`"$PACKAGE_MANAGER" list installed | grep "redhat-rpm-config" | wc -l`
if [ $IS_SUSE -eq 0 ] && [ $REDHAT_RPM_CONFIG -eq 0 ]; then
	echo "Install redhat-rpm-config first!"
	exit -1
fi;

if [ $IS_CENTOS_6_X -gt 0 ]; then
	CENTOS_DEVTOOLS_1_1=`yum list installed | grep "devtoolset-1.1" | wc -c`
	if [ $CENTOS_DEVTOOLS_1_1 -eq 0 ]; then
	        echo "Install devtools 1.1 first. CentOS 6.X needs it to compile C++ V11 code."
	        echo "Execute: 'cd /etc/yum.repos.d;wget http://people.centos.org/tru/devtools-1.1/devtools-1.1.repo';"
	        echo "Then run: 'yum install devtoolset-1.1-gcc devtoolset-1.1-gcc-c++'"
	        exit -1
	fi;
	echo -e "\nIMPORTANT:\n"
	echo "if the build fails with a message similar to these below, execute the line 'scl enable devtoolset-1.1 bash' prior to running this script"
	echo "cc1plus: error: unrecognized command line option -std=c++11"
	echo -e "cc1plus: error: unrecognized command line option -std=gnu++11\n"
fi;

CURRENT_DIR=`pwd`

IS_FEDORA=`cat /etc/*-release | grep -i "fedora"| wc -c`
if [ $IS_CENTOS -ne 0 ] || [ $IS_FEDORA -ne 0 ]; then
	export IS_CENTOS_OR_FEDORA=1
fi

if [ $IS_CENTOS -ne 0 ] && [ $IS_CENTOS_6_X -eq 0 ]; then
	SPEC_FILE=tissuestack_sysctl.spec
fi

if [ $IS_FEDORA -ne 0 ]; then
    SPEC_FILE=tissuestack_sysctl.spec
    export USES_SYSTEMCTL=1
fi

if [ $IS_SUSE -ne 0 ]; then
    SPEC_FILE=tissuestack_suse.spec
fi;

PKI_BUILD=0
if [ $# -ne 0 ]; then
    if [ "$1" == "CLIENTS" ]; then
        TARGET=clients
        SPEC_FILE=tissuestack_clients.spec
    else
        PKI_BUILD=1
    fi;
fi;

echo "Calling TissueStack make with target $TARGET"
cd $CURRENT_DIR/src/c++;make $TARGET
if [ $? -ne 0 ]; then
	echo "Build was NOT successful"
	exit -1
fi;

echo "Copying spec file and source tar"
cp -f $CURRENT_DIR/rpm/$SPEC_FILE $HOME/rpmbuild/SPECS
cd $HOME/rpmbuild/SPECS/

# alter the standard apache port for PKI build
if [ $PKI_BUILD -ne 0 ]; then
	sed -i "s/APACHE_PORT=80/APACHE_PORT=$1/g" $SPEC_FILE
fi

echo "Calling RPM build now ..."
rpmbuild -bb $SPEC_FILE
