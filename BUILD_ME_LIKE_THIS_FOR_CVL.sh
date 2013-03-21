clear

echo -e "\n\n*********************************************************************************************************"
echo -e "!!! THIS BUILD OF TISSUE STACK BUILDS FOR THE PARTICULAR NEEDS OF CVL e.g MODULES AND README.TXT !!!"
echo -e "*********************************************************************************************************\n\n"

VERSION=1.1
DATA_PATH=/mnt/tissuestack

if [ ! -f /usr/include/gtk-2.0/gtk/gtk.h ]; then echo "ERROR: Could not find gtk development files. Insure the package is installed, e.g. run: 'yum install gtk2-devel"; exit -1
fi

javac &> /dev/null
if [ $? -ne 0 ]; then
	echo -e "\nYou have to install a java sdk, version >= 1.6. e.g 'yum install java-1.6.0-openjdk' might do the job !!!\n"
	exit -1
fi
mvn -v &> /dev/null
if [ $? -ne 0 ]; then
	echo -e "\nYou have to install apache maven, version >= 3 !!!\n"
	exit -1
fi

VERSION_3_LINE=`mvn -v | grep "Maven 3." | wc -l`
if [ $VERSION_3_LINE -ne 1 ]; then 
	echo -e "\n1111You have to install apache maven, version >= 3 !!!\n"; exit -1;
fi

echo -n "Cleaning..."
make -f Makefile.centOS clean > /dev/null
mvn clean > /dev/null
echo -e "Finished Cleaning.\n"

make -f Makefile.centOS dist VERSION=$VERSION DATA_PATH=$DATA_PATH

echo -e "\n\n*******************************************************************"
echo -e "\tNOW GO AHEAD AND CVL BUILD ME FROM THE TAR JUST CREATED"
echo -e "*******************************************************************"
