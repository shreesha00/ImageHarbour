#include "image_harbour_cli.h"

#include "../rpc/common.h"

namespace imageharbour {

std::atomic<uint8_t> ImageHarbourClient::local_rpc_cnt_ = 0;

void imageharbour_cli_sm_handler(int, erpc::SmEventType, erpc::SmErrType, void*) {}

void imageharbour_rpc_cont_func_async(void* _ctx, void* tag) { reinterpret_cast<RPCToken*>(tag)->SetComplete(); }

ImageHarbourClient::ImageHarbourClient() : del_nexus_on_finalize_(true) {}

void ImageHarbourClient::InitializeConn(const Properties& p, const std::string& svr, void* param) {
    {
        std::lock_guard<std::mutex> lock(init_lk_);
        if (!nexus_) {
            const std::string cli_uri = p.GetProperty(PROP_IH_CLI_URI, PROP_IH_CLI_URI_DEFAULT);
            nexus_ = new erpc::Nexus(cli_uri);
            LOG(INFO) << "Nexus bind to " << cli_uri;
        } else {
            del_nexus_on_finalize_ = false;
        }
    }

    // called from clients
    uint8_t local_rpc_id = global_rpc_id_.fetch_add(1) + IH_CLI_RPCID_OFFSET;
    if (!rpc_) {
        const uint8_t phy_port = std::stoi(p.GetProperty("erpc.phy_port", "0"));
        rpc_ = new erpc::Rpc<erpc::CTransport>(nexus_, nullptr, local_rpc_id, imageharbour_cli_sm_handler, phy_port);
        LOG(INFO) << "RPC object created";
    }
    rpc_use_cnt_.fetch_add(1);

    uint64_t image_server_thread_count = std::stoi(p.GetProperty(PROP_THREADCOUNT, PROP_THREADCOUNT_DEFAULT));
    uint8_t remote_rpc_id = local_rpc_cnt_.fetch_add(1) % image_server_thread_count + IH_SVR_RPCID_OFFSET;

    session_num_ = rpc_->create_session(svr, remote_rpc_id);
    while (!rpc_->is_connected(session_num_)) {
        LOG(INFO) << "Connecting to ImageHarbour eRPC server " << svr << "[" << (int)rpc_->get_rpc_id() << "->"
                  << (int)remote_rpc_id << "]";
        RunERPCOnce();
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }

    LOG(INFO) << "Connected to ImageHarbour eRPC server at " << svr << "[" << (int)rpc_->get_rpc_id() << "->"
              << (int)remote_rpc_id << "]";

    const int msg_size = std::stoull(p.GetProperty(PROP_IH_MSG_SIZE, PROP_IH_MSG_SIZE_DEFAULT));
    req_ = rpc_->alloc_msg_buffer_or_die(PAGE_SIZE);
    resp_ = rpc_->alloc_msg_buffer_or_die(msg_size);
}

void ImageHarbourClient::Finalize() {
    rpc_->free_msg_buffer(resp_);
    rpc_->free_msg_buffer(req_);
    if (rpc_use_cnt_.fetch_sub(1) == 1) delete rpc_;
    if (del_nexus_on_finalize_ && global_rpc_id_.fetch_sub(1) == 1) {
        delete nexus_;
        nexus_ = nullptr;
    }
}

void ImageHarbourClient::FetchImageMetadata(const std::string& image_name, std::string& temp) {
    memcpy(req_.buf_, image_name.c_str(), image_name.size());

    rpc_->resize_msg_buffer(&req_, image_name.size());
    RPCToken tkn;
    rpc_->enqueue_request(session_num_, FETCH_IMAGE, &req_, &resp_, imageharbour_rpc_cont_func_async, &tkn);

    while (!tkn.Complete()) {
        RunERPCOnce();
    }

    if (resp_.get_data_size() > sizeof(Status)) {
        temp = std::move(std::string(reinterpret_cast<char*>(resp_.buf_), resp_.get_data_size()));
    }
    return;
}

}  // namespace imageharbour