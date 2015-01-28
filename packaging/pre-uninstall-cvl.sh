#!/bin/bash
rm -f /tmp/uninstall.log
touch /tmp/uninstall.log
chmod 666 /tmp/uninstall.log
if [ $1 -ne 1 ]; then
	/etc/init.d/tissuestack stop &>> /tmp/uninstall.log
fi
exit 0
