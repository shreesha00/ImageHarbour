#!/bin/bash

source commons.sh

IMAGE_NAME="hello-world"
IMAGE_PATH="$TMPFS_DIR/$IMAGE_NAME.tar"

# time_micro workload
sudo ${PROJ_DIR}/build/src/client/disk_client