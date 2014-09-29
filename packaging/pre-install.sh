#!/bin/bash
rm -f /tmp/pre-install.log
touch /tmp/pre-install.log
chmod 666 /tmp/pre-install.log
/etc/init.d/tissuestack stop &>> /tmp/pre-install.log
/etc/init.d/apache2 stop &>> /tmp/pre-install.log
rm -rf /opt/tissuestack/web/* &>> /tmp/pre-install.log
exit 0