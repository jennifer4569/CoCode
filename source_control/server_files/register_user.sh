#!/bin/bash

#in /root/
FILE="/home/shared/user_credentials"
user="olduser"
pass="oldpass"

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

		#deletes user_credentials once done
		rm -rf $FILE
	    fi

	    ((i=i+1))
	done
fi
