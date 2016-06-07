#!/bin/bash

echo "DROP USER '$repl_user'@'%'" | sudo mysql
echo "grant all privileges on *.*  to '$repl_user'@'%' identified by '$repl_password'" 
echo "grant all privileges on *.*  to '$repl_user'@'%' identified by '$repl_password'" | sudo mysql
