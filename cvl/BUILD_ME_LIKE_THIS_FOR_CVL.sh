clear

NAME=tissuestack
VERSION=1.2
BUILD_DIR=/tmp/tissuestack_build
DEST_PATH=${NAME^^}/$VERSION

if [ `pwd | grep /usr/local/src/$DEST_PATH | wc -c` -eq 0 ]; then
        echo "Please copy directory contents to '/usr/local/src/$DEST_PATH' and execute this script there!"
        exit -1;
fi

tar xvzf $NAME-$VERSION-sources.tar.gz
if [ $? -ne 0 ]; then
        echo "WARNING: $NAME-$VERSION-sources.tar.gz IS NOT PRESENT! If this is the first run of the script, the build WILL fail later on. If it isn't, you can safely ignore this message."
fi

echo -e "\n\n*************************************************************************************"
echo -e "!!! THIS IS A CUSTOMIZED BUILD OF TISSUESTACK FOR CVL !!!"
echo -e "*************************************************************************************\n\n"


cd sources
./BUILD_ME_LIKE_THIS_FOR_CVL.sh
if [ $? -ne 0 ]; then
        echo "Build aborted! Check for errors..."
        exit -1
fi

cd ..
rm -rf $NAME-$VERSION-sources.tar.gz
rm -rf $NAME-$VERSION-binaries.tar.gz
cp $BUILD_DIR/$NAME-$VERSION.tar.gz $NAME-$VERSION-binaries.tar.gz
rm -rf $BUILD_DIR
