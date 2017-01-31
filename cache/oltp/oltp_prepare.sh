#!/bin/bash

DEFAULT_HOST=127.0.0.1
DEFAULT_PORT=3306
DEFAULT_USER=$USER
DEFAULT_PASSWORD=

DEFAULT_DB=maxscale_oltp
DEFAULT_TABLE_SIZE=100000

function usage
{
    echo "usage: oltp_prepare [-h host] [-P port] [-u user] [-p password] [-d db] [-t table_size] [-?]"
    echo
    echo "-h: The host where MaxScale/MySQL/MariaDB is running, default $DEFAULT_HOST."
    echo "-P: The port to connect to, default $DEFAULT_PORT."
    echo "-u: The user to connect as, default $DEFAULT_USER."
    echo "-p: The password to use, default $DEFAULT_PASSWORD."
    echo "-d: The database to create, default $DEFAULT_DB."
    echo "-t: The table size, default $DEFAULT_TABLE_SIZE."
    echo
    echo "This command will prepare the database for oltp_run.sh."
    echo
}

function create_database
{
    local db=$1
    local host=$2
    local port=$3
    local user=$4
    local password=$5

    echo "CREATE DATABASE IF NOT EXISTS $db" | mysql --host=$host --port=$port --user=$user --password=$password

    if [ $? -ne 0 ]
    then
	echo "error: Could not create database $db."
	exit 1
    fi
}


function prepare_database
{
    local db=$1
    local table_size=$2
    local host=$3
    local port=$4
    local user=$5
    local password=$6

    sysbench --test=oltp --oltp-table-size=$table_size --db-driver=mysql \
             --mysql-host=$host --mysql-port=$port --mysql-user=$user --mysql-password=$password \
	     --mysql-db=$db \
	     prepare
}


function main
{
    local host=$DEFAULT_HOST
    local port=$DEFAULT_PORT
    local user=$DEFAULT_USER
    local password=$DEFAULT_PASSWORD
    local db=$DEFAULT_DB
    local table_size=$DEFAULT_TABLE_SIZE

    while getopts ":d:t:h:P:u:p:?" opt
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

    create_database $db $host $port $user $password

    prepare_database $db $table_size $host $port $user $password
}

main $*
