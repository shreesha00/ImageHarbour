#pragma once

#include <infinity/core/Context.h>
#include <infinity/memory/Buffer.h>
#include <infinity/memory/RegionToken.h>
#include <infinity/queues/QueuePair.h>
#include <infinity/queues/QueuePairFactory.h>
#include <infinity/requests/RequestToken.h>

#include "../rpc/erpc_transport.h"
#include "../rpc/rpc_token.h"
#include "glog/logging.h"

namespace imageharbour {
class ImageHarbourClient : public ERPCTransport {
   public:
    ImageHarbourClient(const Properties &p);

    void InitializeConn(const Properties &p, const std::string &svr, void *param) override;
    void Initialize(const Properties &p) override {
        LOG(ERROR) << "This is a client RPC transport";
        throw Exception("Wrong rpc transport type");
    }
    void Finalize() override;

    void FetchImageMetadata(const std::string &image_name, std::vector<std::pair<uint64_t, uint64_t>> &image_metadata,
                            char *digest, uint64_t &size);

    void FetchImage(std::vector<std::pair<uint64_t, uint64_t>> &image_metadata, uint64_t &size, std::string &filename);

   protected:
    int session_num_;
    static std::atomic<uint8_t> local_rpc_cnt_;

    infinity::core::Context *context_;
    infinity::queues::QueuePairFactory *qp_factory_;
    std::vector<std::string> mem_servers_;
    std::vector<infinity::queues::QueuePair *> qps_;

    bool del_nexus_on_finalize_;

    erpc::MsgBuffer req_;
    erpc::MsgBuffer resp_;
};

}  // namespace imageharbour