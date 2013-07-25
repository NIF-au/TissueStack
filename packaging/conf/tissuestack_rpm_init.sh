#!/bin/sh

# chkconfig: 345 70 30
# description: TissueStack rudimentary init script (delegating to .../bin/tissuestack where the path depends on the install path)
# processname: tissuestack

start () {
        echo -n "Starting Tissuestack..."
       if [ -d "/mnt/tissuestack/data" ]; then
                su -c 'mkdir -p /mnt/tissuestack/data  >> /tmp/messages' tissuestack
        fi
        if [ -d "/mnt/tissuestack/upload" ]; then
                su -c 'mkdir -p /mnt/tissuestack/upload  >> /tmp/messages' tissuestack
        fi
        if [ -d "/mnt/tissuestack/tiles" ]; then
                su -c 'mkdir -p /mnt/tissuestack/tiles  >> /tmp/messages' tissuestack
        fi
        su -c 'echo "Starting Tissuestack..." >> /tmp/messages;. /etc/profile.d/modules.sh >> /tmp/messages;. /etc/profile.d/tissuestack_modules.sh >> /tmp/messages;tissuestack start >> /tmp/messages' tissuestack >> /var/log/messages
        if [ $? -eq 1 ]; then
                echo "OK"; echo "TissueStack started." >> /var/log/messages
        else
                echo "FAILED";echo "Failed to start TissueStack." >> /var/log/messages
                return -1;
        fi;
        touch /var/lock/subsys/tissuestack >> /var/log/messages
        return 0
}

stop () {
        echo -n "Stopping Tissuestack..."
        echo "Stopping Tissuestack..." >> /var/log/messages
        . /etc/profile.d/modules.sh >> /var/log/messages
        . /etc/profile.d/tissuestack_modules.sh >> /var/log/messages
        tissuestack stop >> /var/log/messages
        if [ $? -eq 1 ]; then
                echo "OK"; echo "TissueStack stopped." >> /var/log/messages
        else
                echo "FAILED";echo "Failed to stop TissueStack." >> /var/log/messages
                return -1;
        fi;
        rm -rf /var/lock/subsys/tissuestack >> /var/log/messages
        rm -rf /tmp/tissue_stack_communication >> /var/log/messages
        return 0;
}

case "$1" in
  start)
        start
        ;;
  stop)
        stop
        ;;
  restart)
        stop
        start
        ;;
  *)
        echo $"Usage: $0 {start|stop|restart}"
esac