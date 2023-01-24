#!/bin/bash

if [ -f $HOME/.bash_profile ] ; then
	. $HOME/.bash_profile
fi

session=${1:-render}
echo "session=$session"
case $session in
	render) 
		xset s off -dpms
		xsetroot -gray
#		xrandr --output DVI-I-1 --rate 120 --reflect y --mode 1920x1080
#		run-render &
		exec render-session
		;;
	icewm)
		exec icewm-session
		;;
	*) 
		echo "Unknown session type. Use 'render' (the default) or 'icewm'."
		;;
esac

