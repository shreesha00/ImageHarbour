# ImageHarbour

## Installation

### eRPC
The [eRPC](https://github.com/erpc-io/eRPC.git) library must be installed in the parent directory of ImageHarbour as follows
```
git clone https://github.com/erpc-io/eRPC.git
cd eRPC
sudo apt-get install libnuma-dev -y
cmake . -DPERF=ON -DTRANSPORT=infiniband -DROCE=ON -DLOG_LEVEL=info
make -j
```

### Building ImageHarbour
```
sudo apt-get install zlib1g-dev
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```
