#!/bin/sh
### BEGIN INIT INFO
# Provides:          rMeasureService
### END INIT INFO

DAEMON=/home/repara/RMeasureService/rMeasureService
DAEMONARGS=" -c /home/repara/RMeasureService/rMeasureService.cfg"
. /lib/lsb/init-functions

start_rMeasureService_daemon() {
	start-stop-daemon --start --quiet --exec $DAEMON --$DAEMONARGS
}

stop_rMeasureService_daemon() {
	start-stop-daemon --stop --quiet --signal TERM --oknodo --exec $DAEMON
}

case "$1" in
  start)
        log_daemon_msg "Starting distributed compiler daemon" "rMeasureService"
        start_rMeasureService_daemon
        log_end_msg $?
        ;;
  stop)
        log_daemon_msg "Stopping distributed compiler daemon" "rMeasureService"
        stop_rMeasureService_daemon
        log_end_msg $?
        ;;
  restart|force-reload)
        log_daemon_msg "Restarting distributed compiler daemon" "rMeasureService"
        stop_rMeasureService_daemon
        sleep 1
        start_rMeasureService_daemon
        log_end_msg $?
        ;;
  status)
        status_of_proc "$DAEMON" "rMeasureService" && exit 0 || exit $?
        ;;
  *)
        N=/etc/init.d/rMeasureService
        echo "Usage: $N {start|stop|restart|force-reload|status}" >&2
        exit 1
        ;;
esac

exit 0


