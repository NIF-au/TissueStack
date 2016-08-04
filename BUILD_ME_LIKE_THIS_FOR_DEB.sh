#!/bin/sh
clear

export IS_RELEASE=1
export TISSUESTACK_BUILD_VERSION=2.3

echo "Setting up build directory and copying config files"
BUILD_DIR=/tmp/tissuestack_build_debian
rm -rf $BUILD_DIR

EXT_BUILD_DIR=$BUILD_DIR/tissuestack-$TISSUESTACK_BUILD_VERSION

TARGET=server
if [ $# -ne 0 ] && [ "$1" = "CLIENTS" ]; then
    TARGET=clients
    EXT_BUILD_DIR=$BUILD_DIR/tissuestack-$TARGET-$TISSUESTACK_BUILD_VERSION
fi;

CURRENT_DIR=`pwd`

mkdir -p $EXT_BUILD_DIR/debian
cp -rf $CURRENT_DIR/deb/$TARGET/* $EXT_BUILD_DIR/debian

echo "Calling TissueStack make with target $TARGET"
cd $CURRENT_DIR/src/c++;make $TARGET

echo "Believe it or not we have to wait for tar to finish off..."
sleep 5

echo "Starting DEBIAN package build now ..."
cd $EXT_BUILD_DIR
dpkg-buildpackage -us -uc
