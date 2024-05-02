#pragma once

#include <dirent.h>
#include <infinity/core/Context.h>
#include <infinity/memory/Buffer.h>
#include <infinity/memory/RegionToken.h>
#include <infinity/queues/QueuePair.h>
#include <infinity/queues/QueuePairFactory.h>
#include <infinity/requests/RequestToken.h>
#include <parallel_hashmap/phmap.h>

#include <chrono>
#include <condition_variable>
#include <functional>
#include <map>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <vector>

#include "../rpc/common.h"
#include "../rpc/erpc_transport.h"
#include "glog/logging.h"

namespace imageharbour {
class ImageServer : public ERPCTransport {
   public:
    ImageServer();
    ~ImageServer();

    void InitializeConn(const Properties &p, const std::string &svr, void *param) override {
        LOG(ERROR) << "This is a server RPC transport";
        throw Exception("Wrong rpc transport type");
    }
    void Initialize(const Properties &p) override;
    void Finalize() override;

   protected:
    static void FetchImageHandler(erpc::ReqHandle *req_handle, void *context);  // called from middle man

    static void Put(std::string &image_name);
    static void Get(std::string &image_name, std::vector<std::pair<uint64_t, uint64_t>> &chunks);
    static void fetch_server_func(const Properties &p, int thread_id);

   protected:
    std::vector<std::thread> server_threads_;
    static phmap::parallel_flat_hash_map<std::string, std::vector<std::pair<uint64_t, uint64_t>>> image_map_;
    static std::vector<uint64_t> per_server_allocated_chunk_idx_;
    static std::vector<uint64_t> per_server_chunk_quota_;
    static std::vector<std::string> mem_servers_;
    static std::string folder_path_;
};
}  // namespace imageharbour