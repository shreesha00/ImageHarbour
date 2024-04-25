#pragma once

#include <dirent.h>

#include <chrono>
#include <condition_variable>
#include <functional>
#include <map>
#include <mutex>
#include <shared_mutex>
#include <thread>

#include "../rpc/common.h"
#include "../rpc/erpc_transport.h"
#include "datalog_client.h"
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

    static void fetch_server_func(const Properties &p);

   protected:
    static std::vector<std::thread> server_threads_;
    static std::string folder_path_;
};
}  // namespace imageharbour