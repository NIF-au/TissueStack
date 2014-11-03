#!/bin/sh
clear

export TISSUESTACK_BUILD_VERSION=1.6

CURRENT_DIR=`pwd`

echo "Setting up build directory and copying config files"
BUILD_DIR=/tmp/tissuestack_build_debian
rm -rf $BUILD_DIR
mkdir -p $BUILD_DIR/tissuestack-$TISSUESTACK_BUILD_VERSION/debian
cp -rf $CURRENT_DIR/deb/* $BUILD_DIR/tissuestack-$TISSUESTACK_BUILD_VERSION/debian

echo "Calling TissueStack make with target dist"
cd $CURRENT_DIR/src/c++;make dist

echo "Believe it or not we have to wait for tar to finish off..."
sleep 5

echo "Starting DEBIAN package build now ..."
cd $BUILD_DIR/tissuestack-$TISSUESTACK_BUILD_VERSION
dpkg-buildpackage -us -uc
