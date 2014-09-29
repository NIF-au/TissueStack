%global _enable_debug_package 0
%global debug_package %{nil}
%global __os_install_post /usr/lib/rpm/brp-compress %{nil}
Name:		minc
Version:	2.1
Release:	0%{?dist}
Summary:	The MINC Toolkit

Group:		Graphics
License:	GPLv3+
URL:		https://github.com/BIC-MNI
Source:		%{name}-%{version}.tar.gz	

BuildRequires:	chrpath	hdf5-devel netcdf-devel
Requires:	hdf5 netcdf

%description
The MINC Toolkit, Version 2.1.0.

%prep
echo %{_prefix}
%setup -q

%build
./configure --prefix=/tmp/minc_build%{_prefix}
make clean
make install

%install
cp -r /tmp/minc_build/* %{buildroot}
cp -r %{_builddir}/%{name}-%{version}/usr/* %{buildroot}%{_prefix}
# stupid rpath stripping
for file in %{buildroot}%{_prefix}/bin/*; do echo "Stripping $file"; if [ `file $file | grep -i elf | wc -c` -ne 0 ]; then chrpath --delete $file; fi; done;

%files
%{_prefix}/bin/*
%{_prefix}/lib/*
%{_prefix}/include/*
%{_prefix}/share/perl5/*
%{_prefix}/share/man/*
%{_prefix}/share/doc/*

#%doc

%clean
rm -rf /tmp/minc_build

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig

#%changelog
