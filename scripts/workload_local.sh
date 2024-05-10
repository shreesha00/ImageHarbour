#!/bin/bash

source commons.sh

IMAGE_NAME="hello-world"
IMAGE_PATH="$TMPFS_DIR/$IMAGE_NAME.tar"

workload() {
    echo -n "$IMAGE_NAME $IMAGE_PATH" | sudo socat -t 10 - UNIX-CONNECT:/imageharbour/daemon | cat
    docker load < $IMAGE_PATH
    # docker run $IMAGE_NAME
}

# time_micro workload
sudo ${PROJ_DIR}/build/src/client/local_client