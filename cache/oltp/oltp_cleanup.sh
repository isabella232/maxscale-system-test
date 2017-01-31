#!/bin/bash

DEFAULT_HOST=127.0.0.1
DEFAULT_PORT=3306
DEFAULT_USER=$USER
DEFAULT_PASSWORD=

DEFAULT_DB=maxscale_oltp

function usage
{
    echo "usage: oltp_cleanup [-h host] [-p port] [-u user] [-p password] [-d db] [-?]"
    echo
    echo "-h: The host where MaxScale/MySQL/MariaDB is running, default $DEFAULT_HOST."
    echo "-P: The port to connect to, default $DEFAULT_PORT."
    echo "-u: The user to connect as, default $DEFAULT_USER."
    echo "-p: The password to use, default $DEFAULT_PASSWORD."
    echo "-d: The database to cleanup, default $DEFAULT_DB."
    echo
    echo "This command will cleanup the database."
    echo
}

function cleanup_database
{
    local db=$1
    local host=$2
    local port=$3
    local user=$4
    local password=$5

    sysbench --test=oltp  --db-driver=mysql \
             --mysql-host=$host --mysql-port=$port --mysql-user=$user --mysql-password=$password \
	     --mysql-db=$db \
	     cleanup
}


function main
{
    local host=$DEFAULT_HOST
    local port=$DEFAULT_PORT
    local user=$DEFAULT_USER
    local password=$DEFAULT_PASSWORD
    local db=$DEFAULT_DB

    while getopts ":d:h:P:u:p:?" opt
    do
	case $opt in
	    d)
		db=$OPTARG
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

    cleanup_database $db $host $port $user $password
}

main $*
