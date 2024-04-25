#pragma once

#include "../rpc/erpc_transport.h"
#include "../rpc/rpc_token.h"
#include "glog/logging.h"

namespace imageharbour {
class ImageHarbourClient : public ERPCTransport {
   public:
    ImageHarbourClient();

    void InitializeConn(const Properties &p, const std::string &svr, void *param) override;
    void Initialize(const Properties &p) override {
        LOG(ERROR) << "This is a client RPC transport";
        throw Exception("Wrong rpc transport type");
    }
    void Finalize() override;

    void FetchImageMetadata(const std::string &image_name);

   protected:
    int session_num_;
    static std::atomic<uint8_t> local_rpc_cnt_;
    bool del_nexus_on_finalize_;

    erpc::MsgBuffer req_;
    erpc::MsgBuffer resp_;
};

}  // namespace imageharbour