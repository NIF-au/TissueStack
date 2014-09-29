#!/bin/sh
clear

export TISSUESTACK_BUILD_VERSION=1.5

CURRENT_DIR=`pwd`

echo "Setting up build directory and copying config files"
BUILD_DIR=/tmp/tissuestack_build_debian
rm -rf $BUILD_DIR
mkdir -p $BUILD_DIR/tissuestack-$TISSUESTACK_BUILD_VERSION/debian
cp -rf $CURRENT_DIR/deb/* $BUILD_DIR/tissuestack-$TISSUESTACK_BUILD_VERSION/debian

echo "Calling TissueStack make with target dist"
cd $CURRENT_DIR/src/c++;make dist

echo "Copying over created dist tar.gz"
cp -f /tmp/tissuestack_build/tissuestack-${TISSUESTACK_BUILD_VERSION}.tar.gz $BUILD_DIR

echo "Starting DEBIAN package build now ..."
cd $BUILD_DIR/tissuestack-$TISSUESTACK_BUILD_VERSION
dpkg-buildpackage -us -uc
