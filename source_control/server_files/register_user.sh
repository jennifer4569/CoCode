#!/bin/bash

#in /root/
FILE="/home/shared/user_credentials"
user="olduser"
pass="oldpass"

if [ $# -ge 2 ]
then
    user=$1
    pass=$2
    sudo adduser $user --gecos ",,," --disabled-password
    echo $user":"$pass | sudo chpasswd
    setfacl -m u:$user:--- /bin/chmod
    setfacl -m u:$user:--- /usr/bin/g++
    setfacl -m u:$user:--- /usr/bin/passwd

    #make the .ssh directory so the user can make ssh keys
    mkdir /home/$user/.ssh
    sudo chown -R $user /home/$user/.ssh
    echo "Registered user: " $user
fi
