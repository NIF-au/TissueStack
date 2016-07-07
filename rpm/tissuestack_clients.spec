%global _enable_debug_package 0
%global debug_package %{nil}
%global __os_install_post /usr/lib/rpm/brp-compress %{nil}
Name:		tissuestack-clients
Version:	2.2
Release:	0%{?dist}
Summary:	TissueStack Clients

Group:		Graphics
License:	GPLv3+
URL:		https://github.com/NIF-au/TissueStack
Source:		%{name}-%{version}.tar.gz

BuildRequires:	minc GraphicsMagick-devel
Requires:	minc nifticlib GraphicsMagick dcmtk

%description
Tissue Stack Clients

%prep
tar xvzf /tmp/tissuestack_build/%{name}-%{version}.tar.gz

%build
mkdir -p %{buildroot}
cp -r * %{buildroot}

%files
/etc/profile.d/*
/usr/local/tissuestack/%{version}/*

#%doc

%clean
rm -rf /tmp/tissuestack_build

%postun
/sbin/ldconfig
exit 0

%post
/sbin/ldconfig
exit 0
