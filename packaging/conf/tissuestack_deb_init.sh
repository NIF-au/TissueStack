#!/bin/sh
### BEGIN INIT INFO
# Provides:          tissuestack
# Required-Start:    $remote_fs $syslog
# Required-Stop:     $remote_fs $syslog
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: tissueStack init script
# Description:       tissueStack rudimentary init script (delegating to .../bin/tissuestack where the path depends on the install path).
### END INIT INFO


start () {
        echo -n "Starting Tissuestack..."
        su -c 'echo "Starting Tissuestack..." >> /tmp/messages;. /etc/profile.d/tissuestack_env.sh >> /tmp/messages;tissuestack start >> /tmp/messages' tissuestack >> /var/log/messages
        if [ $? -eq 1 ]; then
                echo "OK"; echo "TissueStack started." >> /var/log/syslog
        else
                echo "FAILED";echo "Failed to start TissueStack." >> /var/log/syslog
                return -1;
        fi;
        touch /var/lock/tissuestack >> /var/log/syslog
        return 0
}

stop () {
        echo -n "Stopping Tissuestack..."
        echo "Stopping Tissuestack..." >> /var/log/syslog
        . /etc/profile.d/tissuestack_env.sh
        tissuestack stop >> /var/log/syslog
        if [ $? -eq 1 ]; then
                echo "OK"; echo "TissueStack stopped." >> /var/log/syslog
        else
                echo "FAILED";echo "Failed to stop TissueStack." >> /var/log/syslog
                return -1;
        fi;
        rm -rf /var/lock/tissuestack >> /var/log/syslog
        rm -rf /tmp/tissue_stack_communication >> /var/log/syslog
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