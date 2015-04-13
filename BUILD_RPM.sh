#!/bin/sh
make distclean 2> /dev/null

#./configure --prefix=/tmp/dcmtk

if [ ! -f config/Makefile.def ]; then
	echo "Configure must have failed. There is no config/Makefile.def"
	exit 1
fi

#replacement to affect build of shared libs
sed -i "/^AR[ ]*=/c\AR = gcc " config/Makefile.def
sed -i "/^ARFLAGS[ ]*=/c\ARFLAGS = -shared -o " config/Makefile.def
sed -i "/^LIBEXT[ ]*=/c\LIBEXT = so " config/Makefile.def
sed -i "/^RANLIB[ ]*=/c\RANLIB = : " config/Makefile.def
sed -i "/^CXXFLAGS[ ]*=/s/$/ -fPIC/" config/Makefile.def
sed -i "/^CFLAGS[ ]*=/s/$/ -fPIC/" config/Makefile.def

make install && make install-lib
if [ $? -ne 0 ]; then
	echo "make/install failed" 
	exit 1
fi
