Name:		minc
Version:	2.1
Release:	0%{?dist}
Summary:	The MINC Toolkit

Group:		Graphics
License:	GPLv3+
URL:		https://github.com/BIC-MNI
Source:		%{name}-%{version}.tar.gz	

#BuildRequires:	
#Requires:	

%description
The MINC Toolkit, Version 2.1.0.

%prep
%setup -q

%build
%configure
make

%install
make install

%clean
rm -rf $RPM_BUILD_ROOT

%files
%doc


%changelog
