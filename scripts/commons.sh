#!/bin/bash

PASSLESS_ENTRY="/users/sgbhat3/.ssh/id_rsa"
USER="sgbhat3"

PROJ_DIR="/proj/cs523-uiuc-sp24-PG0/$USER/ImageHarbour"
LOG_DIR="/data/imageharbor"
LOCAL_LOG_DIR="$PROJ_DIR/logs/"

TMPFS_DIR="/tmp_harbor"
TMPFS_SIZE="16G"

time_micro() {
    start=$(date +%s%N)
    "$@"
    end=$(date +%s%N)

    elapsed=$((($end - $start) / 1000))

    echo "Elapsed time: $elapsed microseconds"
}

reset_tmpfs() {
    sudo rm -rf $TMPFS_DIR/*
}