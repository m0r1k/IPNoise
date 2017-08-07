#!/bin/sh

##
## Roman E. Chechnev
## simple GDB start script
##

DAEMONS=`ps -Af | grep ipnoise_kernel | grep -v grep | grep -v "\[ipnoise_kernel\]" | awk '{print $9}' | sort  | sort -u`
DAEMONS_COUNT=`ps -Af | grep ipnoise_kernel | grep -v grep | grep -v "\[ipnoise_kernel\]" | awk '{print $9}' | sort  | sort -u | wc -l`
DAEMON_PID=""

## function

function start_gdb
{
    ## serach ipnoise exec file
    DAEMON_PID=$1
    if [ "$DAEMON_PID" == "" ];then
        echo "internal error, empty pid"
        exit 1;
    fi
    if [ ! -r "/proc/$DAEMON_PID" ];then
        echo "there are no process with PID: '$DAEMON_PID'"
        exit 1;
    fi
    gdb /proc/$DAEMON_PID/exe $DAEMON_PID
}

## main

if [ "$DAEMONS_COUNT" == "" ] || [ "$DAEMONS_COUNT" == "0" ];then
    echo "Looks like there are no IPNoise daemons started";
    exit 1;
fi

## if one daemon
if [ "$DAEMONS_COUNT" == "1" ];then
    DAEMON_PID=`ps -Af | grep "$DAEMONS" | grep -v grep  | head -n 1 | awk '{print $2}'`
    start_gdb $DAEMON_PID
    exit 0;
fi

## if more
echo "Looks like more than one IPNoise daemon started:"
echo -e "PID\tDAEMON"
for i in $DAEMONS;do
    ps -Af | grep "$i" | grep -v grep  | head -n 1 | awk '{print $2"\t"$8" "$9}'
done
echo -n "Please enter PID: "
read DAEMON_PID
start_gdb $DAEMON_PID

exit 0;

