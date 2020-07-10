#!/bin/bash
#in /root/

CMD="/home/shared/"

mkdir -p "CMD"

#waits until a file is moved to the $CMD directory, before calling register_user.sh
while :
do
    inotifywait -q -e moved_to "$CMD" >/dev/null
    ./register_user.sh
done
