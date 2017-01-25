#!/bin/bash

script=`basename "$0"`

if [ $# -lt 1 ]
then
    echo "usage: $script <test-name>"
    echo ""
    echo "where <test-name> is the name of the test. That selects the"
    echo "configuration template to be used."
    exit 1
fi

source=masking/$1/masking_rules.json
target=vagrant@$maxscale_IP:/home/vagrant/masking_rules.json

scp -i $maxscale_keyfile -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null $source $target

if [ $? -ne 0 ]
then
    echo "error: Could not copy rules file to maxscale host."
    exit 1
fi

echo $source copied to $target

# [Read Connection Listener Master] in cnf/maxscale.maxscale.cnf.template.$1
port=4008

./mysqltest_driver.sh $1 ./masking/$1 $port
