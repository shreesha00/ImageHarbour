#!/bin/bash

set -ex
source cluster.sh

clients=("0")

reset_client_logs() {
    for ((i = 0; i < ${#mem_svr[@]}; i++)); do
        node="${clients[i]}"
        ssh -i ${PASSLESS_ENTRY} $USER@node$node \
            "sudo rm -rf $LOG_DIR && sudo mkdir -p $LOG_DIR && sudo chmod 777 $LOG_DIR"
    done
}

run_workload() {
    for ((i = 0; i < ${#clients[@]}; i++)); do
        node="${clients[i]}"
        ssh -i ${PASSLESS_ENTRY} $USER@node$node \
            "sudo $PROJ_DIR/build/src/client/basic_cli -P $PROJ_DIR/cfg/properties.prop -P $PROJ_DIR/cfg/rdma.prop > \
             $LOG_DIR/client_$node_$i.log 2>&1 &"
    done
}

collect_client_logs() {
    for ((i = 0; i < ${#clients[@]}; i++)); do
        node="${clients[i]}"
        scp -i ${PASSLESS_ENTRY} $USER@node$node:$LOG_DIR/client_$node_$i.log $LOCAL_LOG_DIR
    done
    reset_client_logs 
}

reset_client_logs
fresh_cluster
run_workload
wait
after_work
collect_client_logs
