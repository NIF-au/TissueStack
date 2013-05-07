#!/bin/bash
clear

echo -e "\n\n*********************************************************************************************************"
echo -e "!!! THIS BUILD OF TISSUE STACK BUILDS FOR THE PARTICULAR NEEDS OF CVL e.g MODULES AND README.TXT !!!"
echo -e "*********************************************************************************************************\n\n"

VERSION=1.1
#DATA_PATH=/mnt/tissuestack
DATA_PATH=/opt/tissuestack

source packaging/conf/tissuestack_modules.sh $VERSION

echo -n "Cleaning..."
make -f Makefile.centOS clean > /dev/null
mvn clean > /dev/null
echo -e "Finished Cleaning.\n"

make -f Makefile.centOS dist VERSION=$VERSION DATA_PATH=$DATA_PATH
if [ $? -ne 0 ]; then
exit -1
fi

echo -e "\n\n*******************************************************************"
echo -e "\tNOW GO AHEAD AND CVL BUILD ME FROM THE TAR JUST CREATED"
echo -e "*******************************************************************"
