#!/bin/bash

load_buffer_cache() {
    cat $1 > /dev/null
}

drop_buffer_cache() {
    echo 3 > /proc/sys/vm/drop_caches
}

drop_disk() {
    drop_buffer_cache
    sudo rm $1
}