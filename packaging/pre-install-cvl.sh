#!/bin/bash
rm -f /tmp/pre-install.log
touch /tmp/pre-install.log
chmod 666 /tmp/pre-install.log
/etc/init.d/tissuestack stop &>> /tmp/pre-install.log
service httpd stop  &>> /tmp/uninstall.log
#clean directories that don't contain user data
rm -rf /opt/tissuestack/web/* &>> /tmp/pre-install.log
rm -rf /opt/tissuestack/sql &>> /tmp/pre-install.log
rm -rf /opt/tissuestack/conf &>> /tmp/pre-install.log
exit 0
