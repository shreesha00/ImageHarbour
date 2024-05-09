#include "image_harbour_cli.h"

#include "../rpc/common.h"

namespace imageharbour {

std::atomic<uint8_t> ImageHarbourClient::local_rpc_cnt_ = 0;

void imageharbour_cli_sm_handler(int, erpc::SmEventType, erpc::SmErrType, void*) {}

void imageharbour_rpc_cont_func_async(void* _ctx, void* tag) { reinterpret_cast<RPCToken*>(tag)->SetComplete(); }

ImageHarbourClient::ImageHarbourClient(const Properties& p) : del_nexus_on_finalize_(true) {
    context_ = new infinity::core::Context();
    qp_factory_ = new infinity::queues::QueuePairFactory(context_);

    mem_servers_ = SeparateValue(p.GetProperty(PROP_MEM_SERVERS, PROP_MEM_SERVERS_DEFAULT), ',');
    for (auto& svr : mem_servers_) {
        qps_.emplace_back(qp_factory_->connectToRemoteHost(svr.c_str(), MEM_SERVER_PORT));
        LOG(INFO) << "Connected to memory server " << svr;
    }

    scratch_pad_ = new char[5 * SCRATCH_PAD_SIZE];
    scratch_pad_offset_ = 0;

    buf_ = new infinity::memory::Buffer(context_, CACHE_GRANULARITY_MIB * MIB);
}

ImageHarbourClient::~ImageHarbourClient() {
    delete[] scratch_pad_;
    for (auto& qp : qps_) {
        delete qp;
    }
    delete buf_;
    delete qp_factory_;
    delete context_;
}

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

void ImageHarbourClient::FetchImageMetadata(const std::string& image_name) {
    if (image_metadata_cache_.find(image_name) != image_metadata_cache_.end()) {
        return;
    }

    memcpy(req_.buf_, image_name.c_str(), image_name.size());

    rpc_->resize_msg_buffer(&req_, image_name.size());
    RPCToken tkn;
    rpc_->enqueue_request(session_num_, FETCH_IMAGE, &req_, &resp_, imageharbour_rpc_cont_func_async, &tkn);

    while (!tkn.Complete()) {
        RunERPCOnce();
    }

    if (resp_.get_data_size() > sizeof(Status)) {
        std::vector<std::pair<uint64_t, uint64_t>> chunk_info;
        char digest[SHA256_DIGEST_SIZE];
        uint64_t size;

        DeSerializeChunkInfo(chunk_info, digest, size, resp_.buf_);
        image_metadata_cache_[image_name] = std::make_tuple(std::move(chunk_info), std::string(digest), size);
    }
    return;
}

void ImageHarbourClient::StoreImage(const std::string& image_path) {
    // create file using open
    int fd = open(image_path.c_str(), O_CREAT | O_WRONLY, 0777);
    if (fd < 0) {
        LOG(ERROR) << "Failed to open file " << image_path;
    }

    if (scratch_pad_offset_ != write(fd, scratch_pad_, scratch_pad_offset_)) {
        LOG(ERROR) << "Failed to write to file " << image_path;
    }
    close(fd);
}

void ImageHarbourClient::FetchImage(const std::string& image_name) {
    // get metadata if needed
    FetchImageMetadata(image_name);

    uint64_t remaining_size = std::get<2>(image_metadata_cache_[image_name]);
    infinity::requests::RequestToken req_token(context_);

    uint64_t read_size = 0;
    scratch_pad_offset_ = 0;
    for (auto& p : std::get<0>(image_metadata_cache_[image_name])) {
        read_size = std::min(remaining_size, CACHE_GRANULARITY_MIB * MIB);
        qps_[p.second]->read(buf_, 0, static_cast<infinity::memory::RegionToken*>(qps_[p.second]->getUserData()),
                             p.first * CACHE_GRANULARITY_MIB * MIB, read_size, &req_token);
        req_token.waitUntilCompleted();
        memcpy(scratch_pad_ + scratch_pad_offset_, buf_->getData(), read_size);

        scratch_pad_offset_ += read_size;
        remaining_size -= read_size;
    }
}

}  // namespace imageharbour