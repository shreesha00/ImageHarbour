# ImageHarbour

## Installation

### Pre-requisites
```
sudo apt-get install libnuma-dev zlib1g-dev libtbb-dev -y
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
