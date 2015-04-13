make distclean

./configure --prefix=/tmp/dcmtk

replacement to affect build of shared libs
sed -i "/^AR[ ]*=/c\AR = gcc " config/Makefile.def
sed -i "/^ARFLAGS[ ]*=/c\ARFLAGS = -shared -o " config/Makefile.def
sed -i "/^LIBEXT[ ]*=/c\LIBEXT = so " config/Makefile.def
sed -i "/^RANLIB[ ]*=/c\RANLIB = : " config/Makefile.def
sed -i "/^CXXFLAGS[ ]*=/s/$/ -fPIC/" config/Makefile.def
sed -i "/^CFLAGS[ ]*=/s/$/ -fPIC/" config/Makefile.def

make install && make install-lib
