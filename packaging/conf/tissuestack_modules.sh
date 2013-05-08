#!/bin/bash
echo -n "Checking Tissue Stack 'modules' requirements..."
if [ ! -f /etc/profile.d/modules.sh ]; then echo "ERROR: Modules package is not installed !!!!!";exit -1; fi;

/etc/profile.d/modules.sh
module load hdf5/1.8.5 2> /tmp/build_mod_load
CHECK_SIZE=`stat -c%s /tmp/build_mod_load`
if [ $CHECK_SIZE -ne 0 ]; then echo "ERROR: Could not locate needed hdf5 package (1.8.5). Please install it and load it: 'module load hdf5/1.8.5' !!!!!";exit -1
fi

module load netcdf/4.1.1 2> /tmp/build_mod_load
CHECK_SIZE=`stat -c%s /tmp/build_mod_load`
if [ $CHECK_SIZE -ne 0 ]; then
        echo "ERROR: Could not locate needed netcdf package (4.1.1). Please install it and load it: 'module load netcdf/4.1.1' !!!!!";exit -1
fi

module load nifticlib/2.0.0 2> /tmp/build_mod_load
CHECK_SIZE=`stat -c%s /tmp/build_mod_load`
if [ $CHECK_SIZE -ne 0 ]; then echo "ERROR: Could not locate needed nifti package (2.0.0). Please install it and load it: 'module load nifticlib/2.0.0' !!!!!";exit -1
fi

module load minc/2.1.0 2> /tmp/build_mod_load
CHECK_SIZE=`stat -c%s /tmp/build_mod_load`
if [ $CHECK_SIZE -ne 0 ]; then echo "ERROR: Could not locate needed minc package (2.1.0). Please install it and load it: 'module load minc/2.1.0' !!!!!";exit -1
fi

module load graphicsmagick/1.3.18 2> /tmp/build_mod_load
CHECK_SIZE=`stat -c%s /tmp/build_mod_load`
if [ $CHECK_SIZE -ne 0 ]; then echo "ERROR: Could not locate needed graphicsmagick package (1.3.18). Please install it and load it: 'module load graphicsmagick/1.3.18' !!!!!";exit -1
fi

module load libjpeg-turbo 2> /tmp/build_mod_load
CHECK_SIZE=`stat -c%s /tmp/build_mod_load`
if [ $CHECK_SIZE -ne 0 ]; then echo "ERROR: Could not locate needed libjpeg-turbo package (>=1.2.0). Please install it and load it: 'module load libjpeg-turbo' !!!!!";exit -1
fi

module load tissuestack 2> /tmp/build_mod_load
echo -e "Finished.\n"
