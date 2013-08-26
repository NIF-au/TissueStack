%global _enable_debug_package 0
%global debug_package %{nil}
%global __os_install_post /usr/lib/rpm/brp-compress %{nil}
Name:		tissuestack
Version:	1.2
Release:	0%{?dist}
Summary:	The TissueStack Application

Group:		Graphics
License:	GPLv3+
URL:		https://github.com/NIF-au/TissueStack
Source:		%{name}-%{version}.tar.gz	

BuildRequires:	minc nifticlib-devel GraphicsMagick-devel gtk2-devel unixODBC-devel
Requires:	minc nifticlib GraphicsMagick gtk2 unixODBC postgresql-server httpd
Provides:	libTissueStack.so()(64bit) libodbcinst.so()(64bit) libodbc.so()(64bit)

%description
Tissue Stack is an open source web based image viewer which allows the user to view 3D data as 2D cross sections. While at its core it's modelled after GIS web mapping applications, it's intended use is for neuro-imaging. Tissue Stack aims at serving its data as image tiles which can be both pre-tiled in advance or created on the fly by extracting from the original source file. The used file formats will be anything that is supported by the MINC Tool Kit (http://www.bic.mni.mcgill.ca/ServicesSoftware/ServicesSoftwareMincToolKit) as well as the nifti format(http://nifti.nimh.nih.gov/).

%prep
%setup -q

%build
echo "Entering build stage (see: /tmp/%{name}-%{version}-rpm-build.log)"
make clean > /tmp/%{name}-%{version}-rpm-build.log
make dist VERSION=%{version} >> /tmp/%{name}-%{version}-rpm-build.log
echo "Completed build stage"

%install
echo "Entering install stage (see: /tmp/%{name}-%{version}-rpm-build.log)"
cd %{buildroot}; tar xvzf /tmp/%{name}_build/%{name}-%{version}.tar.gz >> /tmp/%{name}-%{version}-rpm-build.log
mv %{buildroot}/pre-install.sh /tmp
mv %{buildroot}/post-install.sh /tmp
mv %{buildroot}/pre-uninstall.sh /tmp
mv %{buildroot}/post-uninstall.sh /tmp
echo "Completed install stage"

%files
/etc/profile.d/*
/usr/local/%{name}/%{version}/*
/opt/%{name}/*

#%doc

%clean
rm -rf /tmp/%{name}_build

%pre
rm -f /tmp/pre-install.log
touch /tmp/pre-install.log
chmod 666 /tmp/pre-install.log
/etc/init.d/tissuestack stop &>> /tmp/pre-install.log
service httpd stop  &>> /tmp/uninstall.log
rm -rf /opt/tissuestack/web/* &>> /tmp/pre-install.log
rm -rf /opt/tissuestack/jdk* &>> /tmp/pre-install.log
rm -rf /opt/tissuestack/apache-tomcat-* &>> /tmp/pre-install.log
rm -rf /tmp/tissue_stack_communication &>> /tmp/pre-install.log
exit 0

%preun
rm -f /tmp/uninstall.log
touch /tmp/uninstall.log
chmod 666 /tmp/uninstall.log
/etc/init.d/tissuestack stop &>> /tmp/uninstall.log
exit 0

%postun
chkconfig --del tissuestack &>> /tmp/uninstall.log
rm -rf /etc/init.d/tissuestack &>> /tmp/uninstall.log
rm -rf /etc/httpd/conf.d/tissuestack.conf &>> /tmp/uninstall.log
mv /etc/httpd/conf.d/welcome.conf.disabled /etc/httpd/conf.d/welcome.conf &>> /tmp/uninstall.log
rm -rf /tmp/tissue_stack_communication &>> /tmp/uninstall.log
service httpd restart &>> /tmp/uninstall.log
/sbin/ldconfig
exit 0

%post
rm -f /tmp/post-install.log
touch /tmp/post-install.log
chmod 666 /tmp/post-install.log
useradd -c "tissuestack" -m -d /opt/tissuestack -s /bin/bash -U tissuestack
chown tissuestack:tissuestack /opt/tissuestack
for dirs in `find /opt/tissuestack/* -prune -type d`;do
	if  [ $dirs = "/opt/tissuestack/tiles" ]; then
		chmod g+wr,o+wr $dirs &>> /tmp/post-install.log
		chown tissuestack:tissuestack $dirs &>> /tmp/post-install.log
	else
		chmod -R g+wr,o+wr $dirs &>> /tmp/post-install.log
		chown -R tissuestack:tissuestack $dirs &>> /tmp/post-install.log
		chown -R -H -h tissuestack:tissuestack $dirs &>> /tmp/post-install.log
	fi
done
su -c "su - postgres <<EOF
initdb &>> /tmp/post-install.log
EOF
"
chkconfig postgresql on &>> /tmp/post-install.log
service postgresql start &>> /tmp/post-install.log
sleep 5s
su -c "su - postgres <<EOF
psql -f /opt/tissuestack/sql/create_tissuestack_db.sql &>> /tmp/post-install.log
psql -f /opt/tissuestack/sql/create_tissuestack_tables.sql tissuestack &>> /tmp/post-install.log
psql -f /opt/tissuestack/sql/create_tissuestack_config.sql tissuestack &>> /tmp/post-install.log
psql -f /opt/tissuestack/sql/update_tissuestack_config.sql tissuestack &>> /tmp/post-install.log
EOF
"
chkconfig httpd on &>> /tmp/post-install.log
echo "/opt/tissuestack" > /tmp/escaped.string
sed -i 's/\//\\\//g' /tmp/escaped.string &>> /tmp/post-install.log
ESCAPED_STRING=`cat /tmp/escaped.string` &>> /tmp/post-install.log
cp -f /opt/tissuestack/conf/tissuestack.conf /etc/httpd/conf.d/tissuestack.conf &>> /tmp/post-install.log
sed -i "s/##DOC_ROOT##/$ESCAPED_STRING\/web/g" /etc/httpd/conf.d/tissuestack.conf &>> /tmp/post-install.log
sed -i 's/##ERROR_LOG##/\/var\/log\/httpd\/tissuestack-error.log/g' /etc/httpd/conf.d/tissuestack.conf &>> /tmp/post-install.log
HTTP_VERSION=`httpd -v | grep "Apache/" | cut -f2 -d "/" | cut -f1 -d " " | cut -f1,2 -d "." | sed 's/\.//g'` &>> /tmp/post-install.log
if [ $HTTP_VERSION -gt 23 ]; then sed -i 's/#Require all granted/Require all granted/g' /etc/httpd/conf.d/tissuestack.conf; fi &>> /tmp/post-install.log
mv /etc/httpd/conf.d/welcome.conf /etc/httpd/conf.d/welcome.conf.disabled &>> /tmp/post-install.log
if [ `iptables -S | grep -e "-A INPUT -i lo -j ACCEPT" | wc -c` -eq 0 ]; then
        iptables -I INPUT 1 -i lo -p all -j ACCEPT &>> /tmp/post-install.log
fi
if [ `iptables -S | grep -e "-A INPUT -p tcp -m tcp --dport 8080 -j DROP" | wc -c` -eq 0 ]; then
        iptables -A INPUT -p tcp --destination-port 8080 -j DROP &>> /tmp/post-install.log
fi
if [ `iptables -S | grep -e "-A INPUT -p tcp -m tcp --dport 4242 -j DROP" | wc -c` -eq 0 ]; then
        iptables -A INPUT -p tcp --destination-port 4242 -j DROP &>> /tmp/post-install.log
fi
if [ `iptables -S | grep -e "-A INPUT -p tcp -m tcp --dport 5432 -j DROP" | wc -c` -eq 0 ]; then
        iptables -A INPUT -p tcp --destination-port 5432 -j DROP &>> /tmp/post-install.log
fi
iptables-save &>> /tmp/post-install.log
service httpd restart &>> /tmp/post-install.log
cp -f /opt/tissuestack/conf/tissuestack_init.sh /etc/init.d/tissuestack &>> /tmp/post-install.log
chmod 755 /etc/init.d/tissuestack &>> /tmp/post-install.log
chkconfig --add tissuestack &>> /tmp/post-install.log
chkconfig tissuestack on &>> /tmp/post-install.log
/etc/init.d/tissuestack start &>> /tmp/post-install.log
/sbin/ldconfig
exit 0
