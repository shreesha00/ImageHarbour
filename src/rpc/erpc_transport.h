#pragma once

#include <rpc.h>

#include <atomic>
#include <mutex>
#include <unordered_map>

#include "transport.h"

namespace imageharbour {

class ERPCTransport : public RPCTransport {
   public:
    static void RunERPCOnce();

   protected:
    static erpc::Nexus *nexus_;
    static std::mutex init_lk_;
    static std::atomic<uint8_t> global_rpc_id_;
    thread_local static erpc::Rpc<erpc::CTransport> *rpc_;
    thread_local static std::atomic<int> rpc_use_cnt_;
};

}  // namespace imageharbour