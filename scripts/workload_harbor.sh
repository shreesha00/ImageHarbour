#!/bin/bash

source commons.sh

IMAGE_NAME="debian"
IMAGE_PATH="$TMPFS_DIR/$IMAGE_NAME.tar"

# workload() {
#     echo -n "$IMAGE_NAME $IMAGE_PATH" | sudo socat -t 10 - UNIX-CONNECT:/imageharbour/daemon | cat
#     # docker load < $IMAGE_PATH
#     # docker run $IMAGE_NAME
# }

# time_micro workload

sudo ${PROJ_DIR}/build/src/client/basic_cli -P $PROJ_DIR/cfg/properties.prop -P $PROJ_DIR/cfg/rdma.prop -p filename=$IMAGE_NAME -p filepath=$IMAGE_PATH -p time=30
