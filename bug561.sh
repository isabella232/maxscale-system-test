#!/bin/bash

rp=`realpath $0`
export test_dir=`dirname $rp`
export test_name=`basename $rp`

$test_dir/non_native_setup $test_name
#$test_dir/configure_maxscale.sh
if [ $? -ne 0 ] ; then 
        echo "configure_maxscale.sh failed"
        exit 1
fi

#echo "Waiting for 15 seconds"
#sleep 15

mariadb_err=`mysql -u$repl_user -p$repl_password -h $repl_000 non_existing_db 2>&1`
maxscale_err=`mysql -u$repl_user -p$repl_password -h $maxscale_IP -P 4006 non_existing_db 2>&1`

maxscale_err1=`mysql -u$repl_user -p$repl_password -h $maxscale_IP -P 4008 non_existing_db 2>&1`
maxscale_err2=`mysql -u$repl_user -p$repl_password -h $maxscale_IP -P 4009 non_existing_db 2>&1`

echo "MariaDB message"
echo "$mariadb_err"
echo " "
echo "Maxscale message from RWSplit"
echo "$maxscale_err"
echo "Maxscale message from ReadConn master"
echo "$maxscale_err1"
echo "Maxscale message from ReadConn slave"
echo "$maxscale_err2"

res=0

#echo "$maxscale_err" | grep "$mariadb_err"
if [ "$maxscale_err" != "$mariadb_err" ] ; then
	echo "Messages are different!"
	echo "MaxScale: $maxscale_err"
	echo "Server: $mariadb_err"
	res=1
else
	echo "Messages are same"
fi

if [ "$maxscale_err1" != "$mariadb_err" ] ; then
        echo "Messages are different!"
	    echo "MaxScale: $maxscale_err1"
	    echo "Server: $mariadb_err"
        res=1
else
        echo "Messages are same"
fi

if [ "$maxscale_err2" != "$mariadb_err" ] ; then
        echo "Messages are different!"
	    echo "MaxScale: $maxscale_err2"
	    echo "Server: $mariadb_err"

        res=1
else
        echo "Messages are same"
fi

$test_dir/copy_logs.sh bug561
exit $res
