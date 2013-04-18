#!/bin/sh
clear

if [ $# -eq 0 ]; then
	echo "No Version Number supplied. Please call the script like this: BUILD_ME_LIKE_THIS_FOR_RPM.sh 1.1"
	exit -1
fi

CURRENT_DIR=`pwd`

SOURCE_TAR_DIR=/tmp/tissuestack-$1.tar.gz
SOURCE_TAR_DIR_CONTENT=$SOURCE_TAR_DIR/tissuestack-$1

rm -rf $SOURCE_TAR_DIR
mkdir -p $SOURCE_TAR_DIR_CONTENT

cp -r data $SOURCE_TAR_DIR_CONTENT
cp -r packaging $SOURCE_TAR_DIR_CONTENT
cp -r src $SOURCE_TAR_DIR_CONTENT
cp Makefile $SOURCE_TAR_DIR_CONTENT
cp pom.xml $SOURCE_TAR_DIR_CONTENT
rm -rf */.git
rm -rf */*/.git

cd $SOURCE_TAR_DIR
echo "Building source tar.gz (see file list: /tmp/tissuestack-$1.tar.gz.log)"
tar cvzf tissuestack-$1.tar.gz tissuestack-$1 > /tmp/tissuestack-$1.tar.gz.log
cd $CURRENT_DIR

echo "Setting up build directory and copying config files and source over (see /tmp/tissuestack-$1_deb_build.log)"
BUILD_DIR=/tmp/tissuestack-$1_debian
rm -rf $BUILD_DIR
mkdir $BUILD_DIR
cd $BUILD_DIR
tar xvzf $SOURCE_TAR_DIR/tissuestack-$1.tar.gz > /tmp/tissuestack-$1_deb_build.log
mkdir -p $BUILD_DIR/tissuestack-$1/debian
cp -rf $CURRENT_DIR/deb/* $BUILD_DIR/tissuestack-$1/debian

echo "Starting DEBIAN package build now ..."
cd $BUILD_DIR/tissuestack-$1
export TISSUESTACK_BUILD_VERSION=$1
dpkg-buildpackage -us -uc
