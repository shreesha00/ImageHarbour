#!/bin/bash

source commons.sh

image_svr=("1")
mem_svr=("2")

rm -rf $LOCAL_LOG_DIR
mkdir -p $LOCAL_LOG_DIR

# start memory server
start_memsvr() {
    for ((i = 0; i < ${#mem_svr[@]}; i++)); do
        node="${mem_svr[i]}"
        ssh -i ${PASSLESS_ENTRY} $USER@node$node \
            "sudo $PROJ_DIR/build/src/memory_server/memorysvr -P $PROJ_DIR/cfg/properties.prop \
            -P $PROJ_DIR/cfg/rdma.prop -p server_id=$i > $LOG_DIR/memsvr_$node.log 2>&1 &"
    done
    sleep 5 # wait for server to be up
}

# start image server
start_imagesvr() {
    for ((i = 0; i < ${#image_svr[@]}; i++)); do
        node="${image_svr[i]}"
        ssh -i ${PASSLESS_ENTRY} $USER@node$node \
            "sudo $PROJ_DIR/build/src/image_server/imagesvr -P $PROJ_DIR/cfg/properties.prop -P $PROJ_DIR/cfg/rdma.prop\
             > $LOG_DIR/imagesvr_$node.log 2>&1 &"
    done
    sleep 5 # wait for server to be up
}

reset_logs() {
    for ((i = 0; i < ${#mem_svr[@]}; i++)); do
        node="${mem_svr[i]}"
        ssh -i ${PASSLESS_ENTRY} $USER@node$node \
            "sudo rm -rf $LOG_DIR && sudo mkdir -p $LOG_DIR && sudo chmod 777 $LOG_DIR"
    done

    for ((i = 0; i < ${#mem_svr[@]}; i++)); do
        node="${image_svr[i]}"
        ssh -i ${PASSLESS_ENTRY} $USER@node$node \
            "sudo rm -rf $LOG_DIR && sudo mkdir -p $LOG_DIR && sudo chmod 777 $LOG_DIR"
    done
}

collect_logs() {
    for ((i = 0; i < ${#mem_svr[@]}; i++)); do
        node="${mem_svr[i]}"
        scp -i ${PASSLESS_ENTRY} $USER@node$node:$LOG_DIR/memsvr_$node.log $LOCAL_LOG_DIR
    done

    for ((i = 0; i < ${#mem_svr[@]}; i++)); do
        node="${image_svr[i]}"
        scp -i ${PASSLESS_ENTRY} $USER@node$node:$LOG_DIR/imagesvr_$node.log $LOCAL_LOG_DIR
    done

    reset_logs
}

tear_down() {
    for ((i = 0; i < ${#mem_svr[@]}; i++)); do
        node="${mem_svr[i]}"
        ssh -i ${PASSLESS_ENTRY} $USER@node$node "sudo pkill memorysvr"
    done

    for ((i = 0; i < ${#mem_svr[@]}; i++)); do
        node="${image_svr[i]}"
        ssh -i ${PASSLESS_ENTRY} $USER@node$node "sudo pkill imagesvr"
    done
}

reset_cluster() {
    tear_down
    reset_logs
    start_memsvr
    start_imagesvr
}

cluster_after_work() {
    collect_logs
    tear_down
}

# reset_logs
# start_memsvr
# start_imagesvr

# sleep 10

# collect_logs
# tear_down