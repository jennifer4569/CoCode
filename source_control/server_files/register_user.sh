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

    echo "Registered user: " $user

else
	if [ -f $FILE ]
	then
	    i=0

	    #reads the first two lines of the file
	    head -2 $FILE |
		while read line
		do
		    #first line is username
		    if [ $i -eq 0 ]
		    then
			user=$line
			
		    #second is the password (bc of scoping issues we have to add user here)
		    else
			pass=$line
			
			sudo adduser $user --gecos ",,," --disabled-password
			echo $user":"$pass | sudo chpasswd
			setfacl -m u:$user:--- /bin/chmod
			#deletes user_credentials once done
			rm -rf $FILE
		    fi

		    ((i=i+1))
		done
	fi
fi
