#!/bin/bash

function usage
{
    echo "usage"
}


function main
{
    let prepare=0
    let cleanup=0

    while getopts ":pc?" opt
    do
	case $opt in
	    p)
		let prepare=1
		echo prepare=$prepare
		;;
	    c)
		let cleanup=1
		echo cleanup=$cleanup
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

    if [ $prepare -eq 1 ]
    then
	echo PREPARE
    fi

    if [ $cleanup -eq 1 ]
    then
	echo CLEANUP
    fi
}


main $*
