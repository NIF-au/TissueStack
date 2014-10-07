%global _enable_debug_package 0
%global debug_package %{nil}
%global __os_install_post /usr/lib/rpm/brp-compress %{nil}
Name:		libpqxx-devel
Version:	4.0
Release:	1%{?dist}
Source:         %{name}-%{version}.tar.gz
Summary:	libpqxx-devel

Group:		Database
License:	GPLv3+
URL:		http://pqxx.org/

Requires:	postgresql-devel

%description
The PQXX postgres C++ connector, Version 4.0.1

%prep
%setup -q

%build
./configure CXXFLAGS=-lpthread --prefix=%{buildroot}%{_prefix}/local/pqxx
make

%install
make install
# stupid rpath stripping
#for file in %{buildroot}%{_prefix}/local/pqxx/bin/*; do if [ `file $file | grep -i elf | wc -c` -ne 0 ]; then chrpath --delete $file; fi; done;

%files
/usr/local/pqxx/bin/*
/usr/local/pqxx/lib/*
/usr/local/pqxx/include/*

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig

#%changelog
