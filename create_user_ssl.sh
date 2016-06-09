#!/bin/bash

echo "DROP USER '$repl_user'@'%'" | sudo mysql
echo "grant all privileges on *.*  to '$repl_user'@'%' identified by '$repl_password' require ssl WITH GRANT OPTION" 
echo "grant all privileges on *.*  to '$repl_user'@'%' identified by '$repl_password' require ssl WITH GRANT OPTION" | sudo mysql
