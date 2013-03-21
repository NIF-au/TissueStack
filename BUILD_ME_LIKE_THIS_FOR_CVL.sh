clear

echo -e "\n\n*********************************************************************************************************"
echo -e "!!! THIS BUILD OF TISSUE STACK BUILDS FOR THE PARTICULAR NEEDS OF CVL e.g MODULES AND README.TXT !!!"
echo -e "*********************************************************************************************************\n\n"

VERSION=1.1
DATA_PATH=/mnt/tissuestack

if [ ! -f /usr/include/gtk-2.0/gtk/gtk.h ]; then echo "ERROR: Could not find gtk development files. Insure the package is installed, e.g. run: 'yum install gtk2-devel"; exit -1
fi

javac -version &> /tmp/java.version
VERSION_1_6_LINE=`grep "1.6" /tmp/java.version | wc -l`
if [ $VERSION_1_6_LINE -ne 1 ]; then
	echo -e "\nYou have to install a java sdk, version >= 1.6. e.g 'yum install java-1.6.0-openjdk-devel' might do the job !!!\n"
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

if [ ! -f /etc/profile.d/modules.sh ]; then echo "ERROR: Modules package is not installed !!!!!"; exit -1; fi;

/etc/profile.d/modules.sh
module load hdf5/1.8.5 2> /tmp/build_mod_load
CHECK_SIZE=`stat -c%s /tmp/build_mod_load`
if [ $CHECK_SIZE -ne 0 ]; then echo "ERROR: Could not locate needed hdf5 package (1.8.5). Please install it and load it: 'module load hdf5/1.8.5' !!!!!"; exit -1
fi

module load netcdf/4.1.1 2> /tmp/build_mod_load
CHECK_SIZE=`stat -c%s /tmp/build_mod_load`
if [ $CHECK_SIZE -ne 0 ]; then
        echo "ERROR: Could not locate needed netcdf package (4.1.1). Please install it and load it: 'module load netcdf/4.1.1' !!!!!"; exit -1
fi

module load nifticlib/2.0.0 2> /tmp/build_mod_load
CHECK_SIZE=`stat -c%s /tmp/build_mod_load`
if [ $CHECK_SIZE -ne 0 ]; then echo "ERROR: Could not locate needed nifti package (2.0.0). Please install it and load it: 'module load nifticlib/2.0.0' !!!!!"; exit -1
fi

module load minc/2.1.0 2> /tmp/build_mod_load
CHECK_SIZE=`stat -c%s /tmp/build_mod_load`
if [ $CHECK_SIZE -ne 0 ]; then echo "ERROR: Could not locate needed minc package (2.1.0). Please install it and load it: 'module load minc/2.1.0' !!!!!"; exit -1
fi

module load graphicsmagick/1.3.18 2> /tmp/build_mod_load
CHECK_SIZE=`stat -c%s /tmp/build_mod_load`
if [ $CHECK_SIZE -ne 0 ]; then echo "ERROR: Could not locate needed graphicsmagick package (1.3.18). Please install it and load it: 'module load graphicsmagick/1.3.18' !!!!!"; exit -1
fi

echo -n "Cleaning..."
make -f Makefile.centOS clean > /dev/null
mvn clean > /dev/null
echo -e "Finished Cleaning.\n"

make -f Makefile.centOS dist VERSION=$VERSION DATA_PATH=$DATA_PATH

echo -e "\n\n*******************************************************************"
echo -e "\tNOW GO AHEAD AND CVL BUILD ME FROM THE TAR JUST CREATED"
echo -e "*******************************************************************"
