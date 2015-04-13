%global _enable_debug_package 0
%global debug_package %{nil}
%global __os_install_post /usr/lib/rpm/brp-compress %{nil}
Name:           dcmtk
Version:        3.6.0
Release:        0%{?dist}
Summary:        The DICOM Toolkit

Group:          Graphics
License:        GPLv3+
URL:            http://dicom.offis.de
Source:         %{name}-%{version}.tar.gz

BuildRequires:  chrpath CharLS-devel
Requires:       CharLS

%description
The DICOM Toolkit, Version 3.6.0.

%prep
%setup -q

%build
./configure --prefix=/tmp/dcmtk_build
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

%install
cp -r /tmp/dcmtk_build/* %{buildroot}
# stupid rpath stripping
for file in %{buildroot}/bin/*; do if [ `file $file | grep -i elf | wc -c` -ne 0 ]; then chrpath --delete $file; fi; done;

%files
%{_prefix}/bin/*
%{_prefix}/lib/*
%{_prefix}/include/*
%{_prefix}/share/perl5/*
%{_prefix}/share/man/*
%{_prefix}/share/doc/*

#%doc

%clean
rm -rf /tmp/dcmtk_build

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig

#%changelog

