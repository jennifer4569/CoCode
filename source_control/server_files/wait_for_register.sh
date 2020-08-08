#!/bin/bash
#in /root/

CMD="/home/shared/"
FILE="/home/shared/user_credentials"

mkdir -p "CMD"

#waits until a file is moved to the $CMD directory, before calling register_user.sh
while :
do
    #inotifywait -q -e moved_to "$CMD" >/dev/null
    inotifywait -q -e modify -e moved_to "$CMD" >/dev/null
    #inotifywait -q -e modify "$CMD" >/dev/null

    user="olduser"
    pass="oldpass"

    i=0

    #reads the first two lines of the file
    head -2 $FILE |
        while read line
        do
            #first line is username
            if [ $i -eq 0 ]
            then
                user="$line"

                #second is the password (bc of scoping issues we have to add user here)
            else
                pass="$line"
		echo "Detected request to add user [$user]"
		./sql_register.exe "$user" "$pass"
                #deletes user_credentials once done
                rm -rf $FILE
            fi

            ((i=i+1))
        done
done
