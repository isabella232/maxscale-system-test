#!/bin/bash
for ((i=0 ; i<100 ; i++)) ; 
do
	mysql --host=$maxscale_IP -P 4006 -u $repl_user -p$repl_password --verbose --force --unbuffered=true --disable-reconnect > /dev/null < $test_dir/session_hang/setmix.sql >& /dev/null
done

