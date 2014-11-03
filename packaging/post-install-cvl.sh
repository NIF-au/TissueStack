#!/bin/bash
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
sed -i 's/;exit -1//g' /opt/tissuestack/conf/tissuestack_modules.sh &>> /tmp/post-install.log
cp -r /opt/tissuestack/conf/tissuestack_modules.sh /etc/profile.d/tissuestack_modules.sh &>> /tmp/post-install.log
mkdir -p /mnt/tissuestack/data &>> /tmp/post-install.log
mkdir -p /mnt/tissuestack/tiles &>> /tmp/post-install.log
mkdir -p /mnt/tissuestack/upload &>> /tmp/post-install.log
chown -R tissuestack:tissuestack /mnt/tissuestack/data
chown -R tissuestack:tissuestack /mnt/tissuestack/upload
chown tissuestack:tissuestack /mnt/tissuestack/tiles
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
cp -f /opt/tissuestack/conf/tissuestack.conf /etc/httpd/conf.d/tissuestack.conf &>> /tmp/post-install.log
sed -i "s/##DOC_ROOT##/$ESCAPED_STRING\/web/g" /etc/httpd/conf.d/tissuestack.conf &>> /tmp/post-install.log
sed -i 's/##ERROR_LOG##/\/var\/log\/httpd\/tissuestack-error.log/g' /etc/httpd/conf.d/tissuestack.conf &>> /tmp/post-install.log
HTTP_VERSION=`httpd -v | grep "Apache/" | cut -f2 -d "/" | cut -f1 -d " " | cut -f1,2 -d "." | sed 's/\.//g'` &>> /tmp/post-install.log
if [ $HTTP_VERSION -gt 23 ]; then sed -i 's/#Require all granted/Require all granted/g' /etc/httpd/conf.d/tissuestack.conf; fi &>> /tmp/post-install.log
mv /etc/httpd/conf.d/welcome.conf /etc/httpd/conf.d/welcome.conf.disabled &>> /tmp/post-install.log
if [ `iptables -S | grep -e "-A INPUT -i lo -j ACCEPT" | wc -c` -eq 0 ]; then
        iptables -I INPUT 1 -i lo -p all -j ACCEPT &>> /tmp/post-install.log
fi
if [ `iptables -S | grep -e "-A INPUT -p tcp -m tcp --dport 4242 -j DROP" | wc -c` -eq 0 ]; then
        iptables -A INPUT -p tcp --destination-port 4242 -j DROP &>> /tmp/post-install.log
fi
if [ `iptables -S | grep -e "-A INPUT -p tcp -m tcp --dport 5432 -j DROP" | wc -c` -eq 0 ]; then
        iptables -A INPUT -p tcp --destination-port 5432 -j DROP &>> /tmp/post-install.log
fi
iptables-save &>> /tmp/post-install.log
service httpd restart &>> /tmp/post-install.log
source /etc/profile.d/tissuestack_modules.sh &>> /tmp/post-install.log
cp -f /opt/tissuestack/conf/tissuestack_init.sh /etc/init.d/tissuestack &>> /tmp/post-install.log
chmod 755 /etc/init.d/tissuestack &>> /tmp/post-install.log
chkconfig --add tissuestack &>> /tmp/post-install.log
chkconfig tissuestack on &>> /tmp/post-install.log
/etc/init.d/tissuestack start &>> /tmp/post-install.log
/sbin/ldconfig
exit 0
