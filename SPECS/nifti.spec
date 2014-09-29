%global _enable_debug_package 0
%global debug_package %{nil}
%global __os_install_post /usr/lib/rpm/brp-compress %{nil}
Name:		nifticlib
Version:	2.0
Release:	0%{?dist}
Summary:	The NIFTI C Library

Group:		Graphics
License:	GPLv3+
URL:		http://niftilib.sourceforge.net

BuildRequires:	chrpath
Requires:	zlib 

%description
The NIFTI C Library, Version 2.0.0.

%prep
%setup -q

%build
rm -rf /tmp/nifticlib
make clean
make install

%install
mkdir -p %{buildroot}%{_prefix}
cp -r /tmp/nifticlib/%{version}.%{release}/* %{buildroot}%{_prefix}
# stupid rpath stripping
for file in %{buildroot}%{_prefix}/bin/*; do echo "Stripping $file"; if [ `file $file | grep -i elf | wc -c` -ne 0 ]; then chrpath --delete $file; fi; done;

%files
%{_prefix}/bin/*
%{_prefix}/lib/*
%{_prefix}/include/*

#%doc

%clean
rm -rf /tmp/nifticlib

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig

#%changelog
