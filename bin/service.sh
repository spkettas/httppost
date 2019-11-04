#!/bin/sh
#
# @brief Program start shell
# @date 2019/07/21
# aotain.co.LTD
#


PID=0
PROG="flowmonitor"

Dir=`pwd`
cd $Dir
export LD_LIBRARY_PATH=$Dir/../lib:$LD_LIBRARY_PATH


checkpid(){
	PID=`ps -ef|grep $PROG |grep -v "grep" | grep -v "progmon.sh" | awk -F " " '{print $2}'`

	if [ -z $PID ]; then
		PID=0
	fi
}


start(){
	checkpid

	if [ $PID -eq 0 ]; then
		./$PROG >/dev/null 2>&1 &

		sleep 1
		checkpid

		if [ $PID -ne 0 ]; then
			echo "[OK]:(pid=$PID)"
		else
			echo "[Failed]"
		fi
	else
		echo "INFO:$PROG is running(PID=$PID)"
	fi
}


stop(){
	checkpid

	if [ $PID -eq 0 ]; then
		echo "[OK]:$PROG have stoped"
	else
		echo -n "Stopping $PROG ..."
		kill -9 $PID

		if [ $? -eq 0 ]; then
			echo "[OK]"
		else
			echo "[Failed]"
		fi
	fi
}


restart(){
	stop
	sleep 3
	start
}

status(){
	checkpid

	if [ $PID -ne 0 ]; then
		echo "$PROG"
		echo "---------"
		echo "PID=$PID"
	else
		echo "WARN:$PROG not lanuched"
	fi
}


case "$1" in
   'start')
     start
      ;;
   'stop')
     stop
     ;;
   'restart')
     restart
     ;;
   'status')
     status
     ;;
	*)
     echo "Usage: $0 {start|stop|restart|status}"
     exit 1
esac
