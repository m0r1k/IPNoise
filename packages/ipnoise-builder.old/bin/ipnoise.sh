#!/bin/sh

## Roman E. Chechnev
##
## IPNoise simple start|stop script
VERSION="0.03"
##
## goto ipnoise dir
old_dir=`pwd`
cd `dirname $0`
##
ROOT_DIR=`pwd`
BACKGROUND=
BACKGROUND_ARGS=""

IPNOISE_KERNEL="ipnoise_kernel"
IPNOISE_IMAGE="$ROOT_DIR/ipnoise.img"
LOCK_FILE="$IPNOISE_IMAGE.lock"

rc=0
if [ -n "$BACKGROUND" ];then
    BACKGROUND_ARGS="con0=xterm con1=xterm con=/dev/null"
fi

KERNEL_ARGS="   ubda=$IPNOISE_IMAGE         \
                mem=128M                    \
                con0=fd:0,fd:1 con=pts      \
                rw                          \
                3                           \
                maxcpus=1                   \
                fastboot=1                  \
                speedboot=1                 \
                debug                       \
                eth0=tuntap,tap0,02:00:00:00:00:01,192.168.22.254 \
                $BACKGROUND_ARGS            \
            "

##  get_ipnoise_pids - return IPNoise process and him childs pids
##
function get_ipnoise_pids
{
    IPNOISE_PIDS=`pstree -Ap $IPNOISE_PID | perl -e '
        while (<>){
            push (@pids, $_ =~ /(\d+)/g);
        };
        print join(" ", @pids);
    '`
}

function stop_daemon
{
    IPNOISE_PID="0"
    IPNOISE_PIDS=""

    if [ -r "$LOCK_FILE" ];then
        IPNOISE_PID=`cat "$LOCK_FILE"`
    fi

    if [ -d "/proc/$IPNOISE_PID" ];then
        get_ipnoise_pids
    fi

    echo -n "Stoping.. "
    if [ -n "${IPNOISE_PIDS}" ];then
        ## request for stop
        kill ${IPNOISE_PIDS} >/dev/null 2>&1
        ## wait for stop
        for i in 1 2 3 4 5;do
                get_ipnoise_pids
                if [ ! -n "${IPNOISE_PIDS}" ];then
                        break;
                fi
                echo -n "."
                sleep 1;
        done
        echo
        ## if still up kill it
        get_ipnoise_pids
        if [ -n "${IPNOISE_PIDS}" ];then
                echo "killing pids: ${IPNOISE_PIDS}"
                kill -9 "${IPNOISE_PIDS}" >/dev/null 2>&1
        fi
    else
        echo "[FAILED]"
        echo "No running IPNoise daemon was found"
        rc=1
    fi
    rm -f "$LOCK_FILE"
}

function start_daemon
{
    echo -n "Starting.. "

    IPNOISE_PID="0"
    IPNOISE_PIDS=""

    if [ -r "$LOCK_FILE" ];then
        IPNOISE_PID=`cat "$LOCK_FILE"`
    fi

    if [ -d "/proc/$IPNOISE_PID" ];then
        get_ipnoise_pids
        echo "[Failed]"
        echo "IPNoise daemon already running, PIDS: ${IPNOISE_PIDS}"
        rc=1
        return
    fi

    if [ -n "$BACKGROUND" ];then 
        /bin/bash -c "${ROOT_DIR}/${IPNOISE_KERNEL} ${KERNEL_ARGS}" &
        IPNOISE_PID=`jobs -p`
        echo "$IPNOISE_PID" > $LOCK_FILE
        get_ipnoise_pids
        if [ -n "${IPNOISE_PIDS}" ];then
            echo "Success (pids: ${IPNOISE_PIDS})"
        else
            echo "Failed"
            rc=1
        fi
    else
        /bin/bash -c '
            sleep 2;
            a=;
            _pstree=`pstree -Alcp $PPID`;
            echo "$_pstree" | while read line; do
                if [ -n "$a" ];then
                    IPNOISE_PID=`echo "$line" | sed -e "s/[()]/\n/g" | grep "^[0-9]" | head -n 1`;
                    echo "$IPNOISE_PID" > '$LOCK_FILE';
                    break;
                fi;
                a=1;
            done;
        ' &
        /bin/bash -c "${ROOT_DIR}/${IPNOISE_KERNEL} ${KERNEL_ARGS}"
    fi
}

# See how we were called.
case "$1" in
    start)
        start_daemon
        ;;

    stop)
        stop_daemon
        ;;

    status)
        IPNOISE_PID="0"
        IPNOISE_PIDS=""

        if [ -r "$LOCK_FILE" ];then
            IPNOISE_PID=`cat "$LOCK_FILE"`
        fi

        if [ -d "/proc/$IPNOISE_PID" ];then
            get_ipnoise_pids
        fi

        if [ -n "${IPNOISE_PIDS}" ];then
            echo "Running (pids: ${IPNOISE_PIDS})"
        else
            echo "IPNoise daemon is not running"
        fi
        ;;
    *)
       echo "Usage: $(basename $0) {start|stop|status}"
       rc=1;
esac

cd $old_dir
exit $rc;

