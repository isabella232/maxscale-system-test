#!/bin/bash

source=masking/masking_smoke_rules.json
target=vagrant@$maxscale_IP:/home/vagrant/masking_rules.json

scp -i $maxscale_keyfile -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null $source $target

if [ $? -ne 0 ]
then
    echo "error: Could not copy rules file to maxscale host."
    exit 1
fi

echo $source copied to $target

# [Read Connection Listener Master] in cnf/maxscale.maxscale.cnf.template.masking_smoke
port=4008

./mysqltest_driver.sh $1 ./masking $port
