#!/bin/bash

source commons.sh

IMAGE_NAME="hello-world"
IMAGE_PATH="$TMPFS_DIR/$IMAGE_NAME.tar"

workload() {
    docker pull $IMAGE_NAME
}

time_micro workload