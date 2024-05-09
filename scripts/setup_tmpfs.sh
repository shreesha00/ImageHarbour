#!/bin/bash

TMPFS_DIR="/tmp_harbor"
TMPFS_SIZE="16G"

if [ -d "$TMPFS_DIR" ]; then
    sudo umount $TMPFS_DIR
    sudo rm -rf $TMPFS_DIR
fi

sudo mkdir -p $TMPFS_DIR
sudo mount -t tmpfs -o size=$TMPFS_SIZE,mode=1777 tmpfs $TMPFS_DIR