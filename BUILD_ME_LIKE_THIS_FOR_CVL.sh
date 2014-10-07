#!/bin/bash
clear

echo -e "\n\n*********************************************************************************************************"
echo -e "!!! THIS BUILD OF TISSUE STACK BUILDS FOR THE PARTICULAR NEEDS OF CVL e.g MODULES AND README.TXT !!!"
echo -e "*********************************************************************************************************\n\n"

source packaging/conf/tissuestack_modules.sh

echo -n "Cleaning..."
make -f Makefile.cvl clean > /dev/null
echo -e "Finished Cleaning.\n"

make -f Makefile.cvl dist
if [ $? -ne 0 ]; then
exit -1
fi

echo -e "\n\n*******************************************************************"
echo -e "\tNOW GO AHEAD AND CVL BUILD ME FROM THE TAR JUST CREATED"
echo -e "*******************************************************************"
