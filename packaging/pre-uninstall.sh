#!/bin/bash
rm -f /tmp/uninstall.log
touch /tmp/uninstall.log
chmod 666 /tmp/uninstall.log
/etc/init.d/tissuestack stop &>> /tmp/uninstall.log
exit 0