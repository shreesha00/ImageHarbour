#!/bin/bash

source cluster.sh

clients=("0")

reset_client_logs() {
    for ((i = 0; i < ${#mem_svr[@]}; i++)); do
        node="${clients[i]}"
        ssh -i ${PASSLESS_ENTRY} $USER@node$node \
            "sudo rm -rf $LOG_DIR && sudo mkdir -p $LOG_DIR && sudo chmod 777 $LOG_DIR"
    done
}

run_daemon() {
    for ((i = 0; i < ${#clients[@]}; i++)); do
        node="${clients[i]}"
        ssh -i ${PASSLESS_ENTRY} $USER@node$node \
            "sudo $PROJ_DIR/build/src/client/client_daemon -P $PROJ_DIR/cfg/properties.prop -P $PROJ_DIR/cfg/rdma.prop \
            > $LOG_DIR/daemon_$node.log 2>&1 &"
    done
}

tear_down_daemon() {
    for ((i = 0; i < ${#clients[@]}; i++)); do
        node="${clients[i]}"
        ssh -i ${PASSLESS_ENTRY} $USER@node$node "sudo pkill client_daemon"
    done
}

run_workload_harbor() {
    for ((i = 0; i < ${#clients[@]}; i++)); do
        node="${clients[i]}"
        ssh -i ${PASSLESS_ENTRY} $USER@node$node \
            "cd $PROJ_DIR/scripts && ./workload_harbor.sh > $LOG_DIR/workload_harbor_${node}_${i}_$1.log 2>&1 &"
    done
}

run_workload_registry() {
    for ((i = 0; i < ${#clients[@]}; i++)); do
        node="${clients[i]}"
        ssh -i ${PASSLESS_ENTRY} $USER@node$node \
            "cd $PROJ_DIR/scripts && ./workload_registry.sh > $LOG_DIR/workload_registry_${node}_$i.log 2>&1 &"
    done
}

run_docker() {
    for ((i = 0; i < ${#clients[@]}; i++)); do
        node="${clients[i]}"
        ssh -i ${PASSLESS_ENTRY} $USER@node$node \
            "docker load < /data/hello-world.tar > $LOG_DIR/client_${node}_$i.log 2>&1 &"
    done
}

collect_client_logs() {
    for ((i = 0; i < ${#clients[@]}; i++)); do
        node="${clients[i]}"
        scp -i ${PASSLESS_ENTRY} $USER@node$node:$LOG_DIR/daemon_$node.log $LOCAL_LOG_DIR
        scp -i ${PASSLESS_ENTRY} $USER@node$node:$LOG_DIR/workload_harbor_${node}_${i}*.log $LOCAL_LOG_DIR
        scp -i ${PASSLESS_ENTRY} $USER@node$node:$LOG_DIR/workload_registry_${node}_${i}*.log $LOCAL_LOG_DIR
    done
    # reset_client_logs 
}

docker_rmi_all() {
    for ((i = 0; i < ${#clients[@]}; i++)); do
        node="${clients[i]}"
        ssh -i ${PASSLESS_ENTRY} $USER@node$node "docker system prune -a -f"
    done
}

docker_rmi_all
# tear_down_daemon
reset_client_logs
reset_cluster
# run_daemon
reset_tmpfs

# for ((j = 0; j < 5; j++)); do
#     docker_rmi_all
#     reset_tmpfs
#     # run_workload_registry
# done
run_workload_harbor 0

wait
# tear_down_daemon
cluster_after_work
collect_client_logs
# reset_tmpfs
docker_rmi_all