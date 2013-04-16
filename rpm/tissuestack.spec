%global _enable_debug_package 0
%global debug_package %{nil}
%global __os_install_post /usr/lib/rpm/brp-compress %{nil}
Name:		tissuestack
Version:	1.1
Release:	0%{?dist}
Summary:	The MINC Toolkit

Group:		Graphics
License:	GPLv3+
URL:		https://github.com/BIC-MNI
Source:		%{name}-%{version}.tar.gz	

BuildRequires:	minc nifticlib-devel GraphicsMagick-devel
Requires:	minc GraphicsMagick

%description
Tissue Stack Image Server for on-the-fly tile generation and minc/nifti to .raw conversion.

%prep
%setup -q

%build
make clean
make dist VERSION=%{version}

%install
cp -r /tmp/%{name}_build/* %{buildroot}
# stupid rpath stripping
#for file in %{buildroot}%{_prefix}/bin/*; do if [ `file $file | grep -i elf | wc -c` -ne 0 ]; then chrpath --delete $file; fi; done;

%files

#%doc

%clean
rm -rf /tmp/%{name}_build

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig

#%changelog