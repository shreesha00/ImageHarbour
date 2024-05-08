#!/bin/bash

source commons.sh

reset_tmpfs() {
    if [ -d "$TMPFS_DIR" ]; then
        sudo umount $TMPFS_DIR
        sudo rm -rf $TMPFS_DIR
    fi

    sudo mkdir -p $TMPFS_DIR
    sudo mount -t tmpfs -o size=$TMPFS_SIZE,mode=1777 tmpfs $TMPFS_DIR    
}

drop_disk_file() {
    sudo rm $1
}