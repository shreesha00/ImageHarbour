#pragma once

#define TBB_PREVIEW_CONCURRENT_LRU_CACHE 1

#include <dirent.h>
#include <infinity/core/Context.h>
#include <infinity/memory/Buffer.h>
#include <infinity/memory/RegionToken.h>
#include <infinity/queues/QueuePair.h>
#include <infinity/queues/QueuePairFactory.h>
#include <infinity/requests/RequestToken.h>
#include <parallel_hashmap/phmap.h>
#include <stdlib.h>

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

    static void Get(std::string &image_name, std::vector<std::pair<uint64_t, uint64_t>> &chunks, char *digest,
                    uint64_t &size, int thread_id);
    static void fetch_server_func(const Properties &p, int thread_id);

    static uint64_t GetFreeChunks(uint64_t node, uint64_t chunk_num);

   protected:
    std::vector<std::thread> server_threads_;
    static std::unordered_map<std::string,
                              std::tuple<std::vector<std::pair<uint64_t, uint64_t>>, std::string, uint64_t>>
        image_cache_;
    static std::shared_mutex image_cache_mutex_;
    static std::vector<uint64_t> per_server_allocated_chunk_idx_;
    static std::vector<std::unique_ptr<std::mutex>> per_server_mutex_;
    static std::vector<uint64_t> per_server_chunk_quota_;
    static std::vector<std::string> mem_servers_;
    static std::string folder_path_;
    static infinity::core::Context *context_;
    static infinity::queues::QueuePairFactory *qp_factory_;
    static std::vector<std::vector<infinity::queues::QueuePair *>> per_thread_qps_;
    static char *scratch_pad_;
};
}  // namespace imageharbour