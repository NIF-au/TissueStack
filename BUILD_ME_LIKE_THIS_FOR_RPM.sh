#!/bin/sh
clear

if [ $? -ne 0 ]; then
	echo "Install the rpm development tools first!"
	exit -1
fi;
if [ $# -eq 0 ]; then
	echo "No Version Number supplied. Please call the script like this: BUILD_ME_LIKE_THIS_FOR_RPM.sh 1.1"
	exit -1
fi

echo "Setting up development tree in $HOME"
rm -rf $HOME/rpmbuild
rpmdev-setuptree

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

cp -f rpm/tissuestack.spec $HOME/rpmbuild/SPECS 
cp -rf $SOURCE_TAR_DIR/tissuestack-$1.tar.gz $HOME/rpmbuild/SOURCES/
cd $HOME/rpmbuild/SPECS/

echo "Calling RPM build now ..."
rpmbuild -bb tissuestack.spec
