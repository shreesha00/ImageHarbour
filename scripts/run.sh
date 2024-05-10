#!/bin/bash

source cluster.sh

clients=("1")

reset_client_logs() {
    for ((i = 0; i < ${#clients[@]}; i++)); do
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

run_workload_local() {
    for ((i = 0; i < ${#clients[@]}; i++)); do
        node="${clients[i]}"
        ssh -i ${PASSLESS_ENTRY} $USER@node$node \
            "cd $PROJ_DIR/scripts && ./workload_local.sh > $LOG_DIR/workload_local_${node}_${i}.log 2>&1 &"
    done
}

run_workload_registry() {
    for ((i = 0; i < ${#clients[@]}; i++)); do
        node="${clients[i]}"
        ssh -i ${PASSLESS_ENTRY} $USER@node$node \
            "cd $PROJ_DIR/scripts && ./workload_registry.sh > $LOG_DIR/workload_registry_${node}_$i.log 2>&1 &"
    done
}

run_workload_disk() {
    for ((i = 0; i < ${#clients[@]}; i++)); do
        node="${clients[i]}"
        # ssh -i ${PASSLESS_ENTRY} $USER@node$node "docker pull pytorch/pytorch && \
        #                                           docker save pytorch/pytorch -o /mydata/imageharbor/pytorch.tar && \
        #                                           docker system prune -af && \
        #                                           sudo sh -c 'echo 3 > /proc/sys/vm/drop_caches'"
        # ssh -i ${PASSLESS_ENTRY} $USER@node$node "docker pull hello-world && \
        #                                           docker save hello-world -o /mydata/imageharbor/hello-world.tar && \
        #                                           docker system prune -af && \
        #                                           sudo sh -c 'echo 3 > /proc/sys/vm/drop_caches'"
        # ssh -i ${PASSLESS_ENTRY} $USER@node$node "docker pull postgres:15 && \
        #                                           docker save postgres:15 -o /mydata/imageharbor/postgres:15.tar && \
        #                                           docker system prune -af && \
        #                                           sudo sh -c 'echo 3 > /proc/sys/vm/drop_caches'"
        # ssh -i ${PASSLESS_ENTRY} $USER@node$node "docker pull debian && \
        #                                           docker save debian -o /mydata/imageharbor/debian.tar && \
        #                                           docker system prune -af && \
        #                                           sudo sh -c 'echo 3 > /proc/sys/vm/drop_caches'"
        ssh -i ${PASSLESS_ENTRY} $USER@node$node "docker pull alpine && \
                                                  docker save alpine -o /mydata/imageharbor/alpine.tar && \
                                                  docker system prune -af && \
                                                  sudo sh -c 'echo 3 > /proc/sys/vm/drop_caches'"
        ssh -i ${PASSLESS_ENTRY} $USER@node$node \
            "cd $PROJ_DIR/scripts && ./workload_disk.sh > $LOG_DIR/workload_disk_${node}_$i.log 2>&1 &"
    done
}

run_docker() {
    for ((i = 0; i < ${#clients[@]}; i++)); do
        node="${clients[i]}"
        ssh -i ${PASSLESS_ENTRY} $USER@node$node \
            "docker load < /mydata/hello-world.tar > $LOG_DIR/client_${node}_$i.log 2>&1 &"
    done
}

collect_client_logs() {
    for ((i = 0; i < ${#clients[@]}; i++)); do
        node="${clients[i]}"
        scp -i ${PASSLESS_ENTRY} $USER@node$node:$LOG_DIR/daemon_$node.log $LOCAL_LOG_DIR
        scp -i ${PASSLESS_ENTRY} $USER@node$node:$LOG_DIR/workload_harbor_${node}_${i}*.log $LOCAL_LOG_DIR
        scp -i ${PASSLESS_ENTRY} $USER@node$node:$LOG_DIR/workload_registry_${node}_${i}*.log $LOCAL_LOG_DIR
        scp -i ${PASSLESS_ENTRY} $USER@node$node:$LOG_DIR/workload_local_${node}_${i}*.log $LOCAL_LOG_DIR
        scp -i ${PASSLESS_ENTRY} $USER@node$node:$LOG_DIR/workload_disk_${node}_${i}*.log $LOCAL_LOG_DIR
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
# reset_cluster
# run_daemon
reset_tmpfs

# for ((j = 0; j < 5; j++)); do
#     docker_rmi_all
#     reset_tmpfs
#     # run_workload_registry
# done
echo "====================start workload===================="
# run_workload_harbor 0
run_workload_local
# run_workload_registry
# run_workload_disk

wait
# tear_down_daemon
# cluster_after_work
collect_client_logs
reset_tmpfs
docker_rmi_all