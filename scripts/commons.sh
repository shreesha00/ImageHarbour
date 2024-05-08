#!/bin/bash

PASSLESS_ENTRY="/users/JiyuHu23/.ssh/dassl_rsa"
USER="JiyuHu23"

PROJ_DIR="/proj/rasl-PG0/$USER/imageHarbour"
LOG_DIR="/mydata/imageharbor"
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