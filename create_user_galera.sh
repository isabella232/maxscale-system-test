#!/bin/bash

echo "DROP USER '$galera_user'@'%'" | sudo mysql
echo "grant all privileges on *.*  to '$galera_user'@'%' identified by '$galera_password'" 
echo "grant all privileges on *.*  to '$galera_user'@'%' identified by '$galera_password'" | sudo mysql
