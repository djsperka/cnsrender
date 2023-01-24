#! /bin/sh
### BEGIN INIT INFO
# Provides:          vpixx
# Required-Start:    $remote_fs $syslog
# Required-Stop:     $remote_fs $syslog
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: vpixx projector control on startup/shutdown
# Description:       This script wakes/sleeps vpixx projector
#                    on startup and shutdown.
### END INIT INFO

# Author: Daniel Sperka <djsperka@ucdavis.edu>
#

# Do NOT "set -e"

# PATH should only include /usr/* if it runs after the mountnfs.sh script
PATH=/sbin:/usr/sbin:/bin:/usr/bin
DESC="vpixx projector startup/shutdown script"
NAME=vpixx
DAEMON=/usr/bin/$NAME
DAEMON_ARGS="--options args"
PIDFILE=/var/run/$NAME.pid
SCRIPTNAME=/etc/init.d/$NAME

# Exit if the package is not installed
[ -x "$DAEMON" ] || exit 0

# init config vars
VPIXX_START_OPTIONS='-a -r'
VPIXX_STOP_OPTIONS='-s'
VPIXX_ENABLED=false

# Read configuration variable file if it is present
[ -r /etc/default/$NAME ] && . /etc/default/$NAME

# Load the VERBOSE setting and other rcS variables
. /lib/init/vars.sh

# Define LSB log_* functions.
# Depend on lsb-base (>= 3.2-14) to ensure that this file is present
# and status_of_proc is working.
. /lib/lsb/init-functions

#
# Function that starts the daemon/service
#
do_start()
{
	$DAEMON $VPIXX_START_OPTIONS
	return 0
}

#
# Function that stops the daemon/service
#
do_stop()
{
	$DAEMON $VPIXX_STOP_OPTIONS
	return 0
}

#
# Function that sends a SIGHUP to the daemon/service
#
do_reload() {
	$DAEMON $VPIXX_START_OPTIONS
	return 0
}

case "$1" in
  start)
	[ "$VERBOSE" != no ] && log_daemon_msg "Starting $DESC" "$NAME"
        if [ "$VPIXX_ENABLED" = "no" ] || [ "$VPIXX_ENABLED" = "false" ]
        then
        	log_warning_msg "Not starting $NAME because VPIXX_ENABLED is '$VPIXX_ENABLED' in /etc/default/$NAME"
        else
		do_start
		case "$?" in
			0|1) [ "$VERBOSE" != no ] && log_end_msg 0 ;;
			2) [ "$VERBOSE" != no ] && log_end_msg 1 ;;
		esac
	fi
	;;
  stop)
	[ "$VERBOSE" != no ] && log_daemon_msg "Stopping $DESC" "$NAME"
	do_stop
	case "$?" in
		0|1) [ "$VERBOSE" != no ] && log_end_msg 0 ;;
		2) [ "$VERBOSE" != no ] && log_end_msg 1 ;;
	esac
	;;
  status)
	$DAEMON -i && exit 0 || exit $?
	;;
  reload|force-reload)
	#
	# If do_reload() is not implemented then leave this commented out
	# and leave 'force-reload' as an alias for 'restart'.
	#
	log_daemon_msg "Reloading $DESC" "$NAME"
	do_reload
	log_end_msg $?
	;;
  restart|force-reload)
	#
	# If the "reload" option is implemented then remove the
	# 'force-reload' alias
	#
	log_daemon_msg "Restarting $DESC" "$NAME"
	do_stop
	case "$?" in
	  0|1)
		do_start
		case "$?" in
			0) log_end_msg 0 ;;
			1) log_end_msg 1 ;; # Old process is still running
			*) log_end_msg 1 ;; # Failed to start
		esac
		;;
	  *)
		# Failed to stop
		log_end_msg 1
		;;
	esac
	;;
  *)
	#echo "Usage: $SCRIPTNAME {start|stop|restart|reload|force-reload}" >&2
	echo "Usage: $SCRIPTNAME {start|stop|status|restart|force-reload}" >&2
	exit 3
	;;
esac

:
