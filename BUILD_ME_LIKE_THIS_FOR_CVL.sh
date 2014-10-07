#!/bin/bash
clear

HAS_PQXX_LIB=`yum list installed | grep "libpqxx-devel" | wc -c`
if [ $HAS_PQXX_LIB -eq 0 ]; then
        echo "Build and install libpqxx first => see tissuestack git branch: libpqxx-devel!"
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
        echo -e "\nIMPORTANT:\n"
        echo "if the build fails with a message similar to these below, execute the line 'scl enable devtoolset-1.1 bash' prior to running this script"
        echo "cc1plus: error: unrecognized command line option -std=c++11"
        echo -e "cc1plus: error: unrecognized command line option -std=gnu++11\n"
fi;

echo -e "\n\n*********************************************************************************************************"
echo -e "!!! THIS BUILD OF TISSUE STACK BUILDS FOR THE PARTICULAR NEEDS OF CVL e.g MODULES AND README.TXT !!!"
echo -e "*********************************************************************************************************\n\n"

source packaging/tissuestack_modules.sh

cd src/c++
echo -n "Cleaning..."
make -f Makefile.cvl clean > /dev/null
echo -e "Finished Cleaning.\n"

make -f Makefile.cvl dist
cd ../..
if [ $? -ne 0 ]; then
exit -1
fi

echo -e "\n\n*******************************************************************"
echo -e "\tNOW GO AHEAD AND CVL BUILD ME FROM THE TAR JUST CREATED"
echo -e "*******************************************************************"
