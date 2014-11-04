#!/bin/bash
if [ $1 -ne 1 ]; then
	chkconfig --del tissuestack &>> /tmp/uninstall.log
	rm -rf /etc/init.d/tissuestack &>> /tmp/uninstall.log
	rm -rf /etc/profile.d/tissuestack_modules.sh &>> /tmp/uninstall.log
	rm -rf /etc/httpd/conf.d/tissuestack.conf &>> /tmp/uninstall.log
	mv /etc/httpd/conf.d/welcome.conf.disabled /etc/httpd/conf.d/welcome.conf &>> /tmp/uninstall.log
	rm -rf /tmp/tissue_stack_communication &>> /tmp/uninstall.log
	service httpd restart &>> /tmp/uninstall.log
fi
/sbin/ldconfig
exit 0
