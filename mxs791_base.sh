#!/bin/bash
$test_dir/non_native_setup $test_name
if [ $? -ne 0 ] ; then 
        echo "configure_maxscale.sh failed"
        exit 1
fi

res=0
echo "Trying RWSplit"
echo "show tables" | mysql -u$maxscale_user -p$maxscale_password -h $maxscale_IP -P 4006 test
if [ $? != 0 ] ; then
        res=1
        echo "Can't connect to DB 'test'"
fi

echo "Trying ReadConn master"
echo "show tables" | mysql -u$maxscale_user -p$maxscale_password -h $maxscale_IP -P 4008 test
if [ $? != 0 ] ; then
        res=1
        echo "Can't connect to DB 'test'"
fi

echo "Trying ReadConn slave"
echo "show tables" | mysql -u$maxscale_user -p$maxscale_password -h $maxscale_IP -P 4009 test
if [ $? != 0 ] ; then
        res=1
        echo "Can't connect to DB 'test'"
fi

exit $res
