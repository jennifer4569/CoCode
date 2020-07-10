#!/bin/bash

#in /home/guest/
CMD="/home/shared/"

if [ $# -lt 2 ]
then
    echo "Need a username and password!"
else
    #writes the user inputted credentials into the shared folder (where wait_for_register.sh is looking)
    echo $1 > user_credentials
    echo $2 >> user_credentials
    mv -f user_credentials /home/shared/
fi
