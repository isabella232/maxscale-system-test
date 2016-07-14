#!/bin/bash

rp=`realpath $0`
export test_dir=`dirname $rp`
export test_name=`basename $rp`

$test_dir/non_native_setup $test_name
if [ $? -ne 0 ] ; then 
        echo "configure_maxscale.sh failed"
        exit 1
fi

echo "show tables" | mysql -u$maxscale_user -p$maxscale_password -h $maxscale_IP -P 4006 test
if [ $? != 0 ] ; then
	res=1
	echo "Can't connect to DB 'test'"
else
	res=0
fi

$test_dir/copy_logs.sh bug791
exit $res
