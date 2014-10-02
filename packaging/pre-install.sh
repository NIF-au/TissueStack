#!/bin/bash
rm -f /tmp/pre-install.log
touch /tmp/pre-install.log
chmod 666 /tmp/pre-install.log
/etc/init.d/tissuestack stop &>> /tmp/pre-install.log
/etc/init.d/apache2 stop &>> /tmp/pre-install.log
#clean directories that don't contain user data
rm -rf /opt/tissuestack/web/* &>> /tmp/pre-install.log
rm -rf /opt/tissuestack/jdk1.6.0_25 &>> /tmp/pre-install.log
rm -rf /opt/tissuestack/apache-tomcat-7.0.35 &>> /tmp/pre-install.log
rm -rf /opt/tissuestack/sql &>> /tmp/pre-install.log
rm -rf /opt/tissuestack/conf &>> /tmp/pre-install.log
exit 0