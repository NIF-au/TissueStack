#!/bin/bash
rm -f /tmp/post-install.log
touch /tmp/post-install.log
chmod 666 /tmp/post-install.log
useradd -c "tissuestack" -m -d /opt/tissuestack -s /bin/bash -U tissuestack &>> /tmp/post-install.log
chown tissuestack:tissuestack /opt/tissuestack &>> /tmp/post-install.log
chown tissuestack:tissuestack /opt/tissuestack &>> /tmp/post-install.log
for dirs in `find /opt/tissuestack/* -prune -type d`;do
	if  [ $dirs = "/opt/tissuestack/tiles" ]; then
		chown tissuestack:tissuestack $dirs &>> /tmp/post-install.log
	else
		chown -R tissuestack:tissuestack $dirs &>> /tmp/post-install.log
		chown -R -H -h tissuestack:tissuestack $dirs &>> /tmp/post-install.log
	fi
done
service postgresql start &>> /tmp/post-install.log
sleep 5s
sudo su - postgres <<EOF
psql -f /opt/tissuestack/sql/create_tissuestack_db.sql &>> /tmp/post-install.log
psql -f /opt/tissuestack/sql/create_tissuestack_tables.sql tissuestack &>> /tmp/post-install.log
psql -f /opt/tissuestack/sql/patches.sql tissuestack &>> /tmp/post-install.log
psql -f /opt/tissuestack/sql/create_tissuestack_config.sql tissuestack &>> /tmp/post-install.log
psql -f /opt/tissuestack/sql/update_tissuestack_config.sql tissuestack &>> /tmp/post-install.log
EOF
echo "/opt/tissuestack" > /tmp/escaped.string
sed -i 's/\//\\\//g' /tmp/escaped.string &>> /tmp/post-install.log
ESCAPED_STRING=`cat /tmp/escaped.string` &>> /tmp/post-install.log
cp -f /opt/tissuestack/conf/tissuestack.conf /etc/apache2/sites-available/tissuestack.conf &>> /tmp/post-install.log
sed -i "s/##DOC_ROOT##/$ESCAPED_STRING\/web/g" /etc/apache2/sites-available/tissuestack.conf &>> /tmp/post-install.log
sed -i 's/##ERROR_LOG##/\/var\/log\/apache2\/tissuestack-error.log/g' /etc/apache2/sites-available/tissuestack.conf &>> /tmp/post-install.log
APACHE_VERSION=`apache2 -v | grep "Apache/" | cut -f2 -d "/" | cut -f1 -d " " | cut -f1,2 -d "." | sed 's/\.//g'` &>> /tmp/post-install.log
if [ $APACHE_VERSION -gt 23 ]; then sed -i 's/#Require all granted/Require all granted/g' /etc/apache2/sites-available/tissuestack.conf; fi &>> /tmp/post-install.log
a2ensite tissuestack.org &>> /tmp/post-install.log
a2dissite 000-default &>> /tmp/post-install.log
a2enmod headers proxy proxy_http &>> /tmp/post-install.log
/etc/init.d/apache2 restart &>> /tmp/post-install.log
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
cp -f /opt/tissuestack/conf/tissuestack_init.sh /etc/init.d/tissuestack &>> /tmp/post-install.log
chmod 755 /etc/init.d/tissuestack &>> /tmp/post-install.log
cd /etc/init.d; update-rc.d tissuestack defaults &>> /tmp/post-install.log
/etc/init.d/tissuestack start &>> /tmp/post-install.log
exit 0
