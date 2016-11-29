#!/bin/bash

echo "DROP USER '$galera_user'@'%'" | sudo mysql
echo "grant all privileges on *.*  to '$galera_user'@'%' identified by '$galera_password' WITH GRANT OPTION" 
echo "grant all privileges on *.*  to '$galera_user'@'%' identified by '$galera_password' WITH GRANT OPTION" | sudo mysql

echo "grant all privileges on *.*  to 'maxskysql'@'%' identified by 'skysql' WITH GRANT OPTION" | sudo mysql
