#!/bin/bash

DEFAULT_HOST=127.0.0.1
DEFAULT_PORT=3306
DEFAULT_USER=$USER
DEFAULT_PASSWORD=

DEFAULT_DB=maxscale_oltp
DEFAULT_TABLE_SIZE=100000

DEFAULT_THREADS=4


#sysbench --test=oltp --oltp-table-size=$table_size --oltp-test-mode=simple --db-driver=mysql --db-ps-mode=disable --mysql-host=$host --mysql-db=$db --mysql-user=$user --mysql-password=$password --oltp-skip-trx=on --oltp-read-only=on --num-threads=8 --mysql-port=$port run
#sysbench --test=oltp --oltp-table-size=$table_size --oltp-test-mode=simple --db-driver=mysql --db-ps-mode=disable --mysql-host=$host --mysql-db=$db --mysql-user=$user --mysql-password=$password --oltp-skip-trx=on --num-threads=4 --mysql-port=$port run

#sysbench --test=oltp --oltp-table-size=$table_size --oltp-test-mode=simple --db-driver=mysql --db-ps-mode=disable --mysql-host=$host --mysql-db=$db --mysql-user=$user --mysql-password=$password --oltp-skip-trx=on --num-threads=32 --mysql-port=$port run

function usage
{
    echo "usage: oltp_run [-h host] [-P port] [-u user] [-p password] [-d db] [-t table_size] [-T threads] [-?]"
    echo
    echo "-h: The host where MaxScale/MySQL/MariaDB is running, default $DEFAULT_HOST."
    echo "-P: The port to connect to, default $DEFAULT_PORT."
    echo "-u: The user to connect as, default $DEFAULT_USER."
    echo "-p: The password to use, default $DEFAULT_PASSWORD."
    echo "-d: The database to create, default $DEFAULT_DB."
    echo "-t: The table size, default $DEFAULT_TABLE_SIZE."
    echo "-T: Number of threads, default $DEFAULT_THREADS."
    echo
    echo "This command will run the oltp benchmark."
    echo
}

function run
{
    local threads=$1
    local db=$2
    local table_size=$3
    local host=$4
    local port=$5
    local user=$6
    local password=$7

    sysbench --test=oltp --db-driver=mysql --db-ps-mode=disable --oltp-test-mode=simple --oltp-skip-trx=on \
             --mysql-host=$host --mysql-port=$port --mysql-user=$user --mysql-password=$password \
	     --mysql-db=$db \
	     --oltp-table-size=$table_size \
	     --num-threads=$threads \
	     run
}

function main
{
    local host=$DEFAULT_HOST
    local port=$DEFAULT_PORT
    local user=$DEFAULT_USER
    local password=$DEFAULT_PASSWORD
    local db=$DEFAULT_DB
    local table_size=$DEFAULT_TABLE_SIZE
    local threads=$DEFAULT_THREADS

    while getopts ":d:t:h:P:u:p:T:?" opt
    do
	case $opt in
	    d)
		db=$OPTARG
		;;
	    t)
		table_size=$OPTARG
		;;
	    h)
		host=$OPTARG
		;;
	    P)
		port=$OPTARG
		;;
	    u)
		user=$OPTARG
		;;
	    p)
		password=$OPTARG
		;;
	    \?)
		if [ "$OPTARG" != "?" ]
		then
		    echo "error: Invalid option: -$OPTARG" >&2
		    echo
		fi
		usage
		exit 1
	esac
    done

    run $threads $db $table_size $host $port $user $password
}

main $*
