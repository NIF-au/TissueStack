%global _enable_debug_package 0
%global debug_package %{nil}
%global __os_install_post /usr/lib/rpm/brp-compress %{nil}
Name:		tissuestack
Version:	2.0
Release:	0%{?dist}
Summary:	The TissueStack Application

Group:		Graphics
License:	GPLv3+
URL:		https://github.com/NIF-au/TissueStack
Source:		%{name}-%{version}.tar.gz	

BuildRequires:	minc GraphicsMagick-devel
Requires:	minc nifticlib GraphicsMagick dcmtk postgresql-server httpd

%description
Tissue Stack is an open source web based image viewer which allows the user to view 3D data as 2D cross sections. While at its core it's modelled after GIS web mapping applications, it's intended use is for neuro-imaging. Tissue Stack aims at serving its data as image tiles which can be both pre-tiled in advance or created on the fly by extracting from the original source file. The used file formats will be anything that is supported by the MINC Tool Kit (http://www.bic.mni.mcgill.ca/ServicesSoftware/ServicesSoftwareMincToolKit) as well as the nifti format(http://nifti.nimh.nih.gov/).

%prep
tar xvzf /tmp/%{name}_build/%{name}-%{version}.tar.gz

%build
mkdir -p %{buildroot}
cp -r * %{buildroot}

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
httpd2 -k stop  &>> /tmp/uninstall.log
#clean directories that don't contain user data
rm -rf /opt/tissuestack/web/* &>> /tmp/pre-install.log
rm -rf /opt/tissuestack/sql &>> /tmp/pre-install.log
rm -rf /opt/tissuestack/conf &>> /tmp/pre-install.log
exit 0

%preun
rm -f /tmp/uninstall.log
touch /tmp/uninstall.log
chmod 666 /tmp/uninstall.log
if [ $1 -ne 1 ]; then
	/etc/init.d/tissuestack stop &>> /tmp/uninstall.log
fi
exit 0

%postun
if [ $1 -ne 1 ]; then
	chkconfig --del tissuestack &>> /tmp/uninstall.log
	rm -rf /etc/init.d/tissuestack &>> /tmp/uninstall.log
	rm -rf /etc/apache2/vhosts.d/tissuestack.conf &>> /tmp/uninstall.log
	httpd2 -k restart &>> /tmp/uninstall.log
fi
/sbin/ldconfig
exit 0

%post
rm -f /tmp/post-install.log
touch /tmp/post-install.log
chmod 666 /tmp/post-install.log
useradd -c "tissuestack" -m -d /opt/tissuestack -s /bin/bash -U tissuestack &>> /tmp/post-install.log
chown tissuestack:tissuestack /opt/tissuestack &>> /tmp/post-install.log
for dirs in `find /opt/tissuestack/* -prune -type d`;do
	if  [ $dirs = "/opt/tissuestack/tiles" ]; then
		chown tissuestack:tissuestack $dirs &>> /tmp/post-install.log
	else
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
psql -f /opt/tissuestack/sql/patches.sql tissuestack &>> /tmp/post-install.log
psql -f /opt/tissuestack/sql/create_tissuestack_config.sql tissuestack &>> /tmp/post-install.log
psql -f /opt/tissuestack/sql/update_tissuestack_config.sql tissuestack &>> /tmp/post-install.log
EOF
"
chkconfig httpd on &>> /tmp/post-install.log
echo "/opt/tissuestack" > /tmp/escaped.string
sed -i 's/\//\\\//g' /tmp/escaped.string &>> /tmp/post-install.log
ESCAPED_STRING=`cat /tmp/escaped.string` &>> /tmp/post-install.log
cp -f /opt/tissuestack/conf/tissuestack.conf /etc/apache2/vhosts.d/tissuestack.conf &>> /tmp/post-install.log
sed -i "s/##DOC_ROOT##/$ESCAPED_STRING\/web/g" /etc/apache2/vhosts.d/tissuestack.conf &>> /tmp/post-install.log
sed -i 's/##ERROR_LOG##/\/var\/log\/apache2\/tissuestack-error.log/g' /etc/apache2/vhosts.d/tissuestack.conf &>> /tmp/post-install.log
HTTP_VERSION=`httpd2 -v | grep "Apache/" | cut -f2 -d "/" | cut -f1 -d " " | cut -f1,2 -d "." | sed 's/\.//g'` &>> /tmp/post-install.log
if [ $HTTP_VERSION -gt 23 ]; then sed -i 's/#Require all granted/Require all granted/g' /etc/httpd/conf.d/tissuestack.conf; fi &>> /tmp/post-install.log
touch /etc/apache2/sysconfig.d/include.conf
if [ `grep headers_module /etc/apache2/sysconfig.d/loadmodule.conf | wc -c` -eq 0 ]; then
	echo "LoadModule headers_module /usr/lib64/apache2-prefork/mod_headers.so" >> /etc/apache2/sysconfig.d/loadmodule.conf
	a2enmod headers
fi;	 
if [ `grep proxy_module /etc/apache2/sysconfig.d/loadmodule.conf | wc -c` -eq 0 ]; then
	echo "LoadModule proxy_module /usr/lib64/apache2-prefork/mod_proxy.so" >> /etc/apache2/sysconfig.d/loadmodule.conf
	a2enmod proxy
fi;	 
if [ `grep proxy_http_module /etc/apache2/sysconfig.d/loadmodule.conf | wc -c` -eq 0 ]; then
	echo "LoadModule proxy_http_module /usr/lib64/apache2-prefork/mod_proxy_http.so" >> /etc/apache2/sysconfig.d/loadmodule.conf
	a2enmod proxy_http
fi;	 
if [ `iptables -S | grep -e "-A INPUT -i lo -j ACCEPT" | wc -c` -eq 0 ]; then
        iptables -I INPUT 1 -i lo -p all -j ACCEPT &>> /tmp/post-install.log
fi
if [ `iptables -S | grep -e "-A INPUT -p tcp -m tcp --dport 4242 -j DROP" | wc -c` -eq 0 ]; then
        iptables -A INPUT -p tcp --destination-port 4242 -j DROP &>> /tmp/post-install.log
fi
if [ `iptables -S | grep -e "-A INPUT -p tcp -m tcp --dport 5432 -j DROP" | wc -c` -eq 0 ]; then
        iptables -A INPUT -p tcp --destination-port 5432 -j DROP &>> /tmp/post-install.log
fi
setsebool -P httpd_can_network_connect 1
setsebool -P httpd_enable_homedirs 1
iptables-save &>> /tmp/post-install.log
httpd2 -k restart &>> /tmp/post-install.log
cp -f /opt/tissuestack/conf/tissuestack_init.sh /etc/init.d/tissuestack &>> /tmp/post-install.log
chmod 755 /etc/init.d/tissuestack &>> /tmp/post-install.log
chkconfig --add tissuestack &>> /tmp/post-install.log
chkconfig tissuestack on &>> /tmp/post-install.log
/etc/init.d/tissuestack start &>> /tmp/post-install.log
/sbin/ldconfig
exit 0
