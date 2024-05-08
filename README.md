# ImageHarbour

## Installation

### Pre-requisites
```
sudo apt-get install libnuma-dev zlib1g-dev libtbb-dev libssl-dev socat -y
```
### eRPC
The [eRPC](https://github.com/erpc-io/eRPC.git) library must be installed in the parent directory of ImageHarbour as follows
```
git clone https://github.com/erpc-io/eRPC.git
cd eRPC
cmake . -DPERF=ON -DTRANSPORT=infiniband -DROCE=ON -DLOG_LEVEL=info
make -j
```

### Building ImageHarbour
```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

### Steps to run

* Running the memory servers
    ```
    sudo build/src/memory_server/memorysvr -P cfg/properties.prop -P cfg/rdma.prop -p server_id=<server_id>
    ```
* Running the image server
    ```
    sudo build/src/image_server/imagesvr -P cfg/properties.prop -P cfg/rdma.prop
    ```
* Start the daemon
    ```
    sudo build/src/client/client_daemon -P cfg/properties.prop -P cfg/rdma.prop
    ```
* Send commands to the daemon. Set a decently large timeout for large images (10 secs)
    ```
    echo -n "<image_name> <image_local_path>" | sudo socat -t 10 - UNIX-CONNECT:/imageharbour/daemon | cat
    ```

