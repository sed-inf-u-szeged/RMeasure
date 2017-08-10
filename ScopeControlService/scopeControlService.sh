#!/bin/sh
### BEGIN INIT INFO
# Provides:          reparaService
### END INIT INFO

DAEMON=/home/repara/ScopeControlService/scopeControlService
DAEMONARGS=" -c /home/repara/ScopeControlService/scopeControlService.cfg"
. /lib/lsb/init-functions

start_reparaService_daemon() {
    start-stop-daemon --start --quiet --exec $DAEMON --$DAEMONARGS
}

stop_reparaService_daemon() {
    start-stop-daemon --stop --quiet --signal TERM --oknodo --exec $DAEMON
}

case "$1" in
  start)
        log_daemon_msg "Starting distributed compiler daemon" "reparaService"
        start_reparaService_daemon
        log_end_msg $?
        ;;
  stop)
        log_daemon_msg "Stopping distributed compiler daemon" "reparaService"
        stop_reparaService_daemon
        log_end_msg $?
        ;;
  restart|force-reload)
        log_daemon_msg "Restarting distributed compiler daemon" "reparaService"
        stop_reparaService_daemon
        sleep 1
        start_reparaService_daemon
        log_end_msg $?
        ;;
  status)
        status_of_proc "$DAEMON" "reparaService" && exit 0 || exit $?
        ;;
  *)
        N=/etc/init.d/reparaService
        echo "Usage: $N {start|stop|restart|force-reload|status}" >&2
        exit 1
        ;;
esac

exit 0


