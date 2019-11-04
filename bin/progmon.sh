#!/bin/sh
#
#
# @brief 定时脚本监控服务的执行
# @date 2019/07/17
#
#

#source /etc/profile

Dir="/home/work/flowmonitor_pf/bin"
Prog="flowmonitor"


checkpid()
{
	PROS=`ps -ef|grep ${Prog}|grep -v "grep"| grep -v "$0" | wc -l`

	if [ $PROS -eq 0 ]; then
	     cd $Dir
	     ./service.sh start
	     sleep 2
	     echo `date +"%Y-%m-%d %H:%M:%S"`" Restart $Prog" >> run.log
	fi
}

#Main
checkpid
