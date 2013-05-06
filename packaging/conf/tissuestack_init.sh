#!/bin/sh

# chkconfig: 345 70 30
# description: TissueStack rudimentary init script (delegating to .../bin/tissuestack where the path depends on the install path)
# processname: tissuestack

start () {
        echo -n "Starting Tissuestack..."
        echo "Starting Tissuestack..." >> /var/log/messages
        source /etc/profile.d/modules.sh >> /var/log/messages
        source /etc/profile.d/tissuestack_modules.sh >> /var/log/messages
        tissuestack start >> /var/log/messages
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
        source /etc/profile.d/modules.sh >> /var/log/messages
        source /etc/profile.d/tissuestack_modules.sh >> /var/log/messages
        tissuestack stop >> /var/log/messages
        if [ $? -eq 1 ]; then
                echo "OK"; echo "TissueStack stopped." >> /var/log/messages
        else
                echo "FAILED";echo "Failed to stop TissueStack." >> /var/log/messages
                return -1;
        fi;
        rm -rf /var/lock/subsys/tissuestack >> /var/log/messages
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