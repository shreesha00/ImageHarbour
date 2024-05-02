#include "image_server.h"

#include "sys/mman.h"

namespace imageharbour {

std::string ImageServer::folder_path_ = "";
phmap::parallel_flat_hash_map<std::string, std::vector<std::pair<uint64_t, uint64_t>>> ImageServer::image_map_;
phmap::parallel_flat_hash_map<uint64_t, uint64_t> ImageServer::image_map_;
std::vector<uint64_t> ImageServer::per_server_allocated_chunk_idx_;
std::vector<uint64_t> ImageServer::per_server_chunk_quota_;
std::vector<std::string> ImageServer::mem_servers_;

void svr_sm_handler(int, erpc::SmEventType, erpc::SmErrType, void *) {}

ImageServer::ImageServer() {}
ImageServer::~ImageServer() {}

void ImageServer::Initialize(const Properties &p) {
    const std::string server_uri = p.GetProperty(PROP_IH_SVR_URI, PROP_IH_SVR_URI);
    nexus_ = new erpc::Nexus(server_uri, 0, 0);

    folder_path_ = p.GetProperty(PROP_IH_FOLDER_PATH, PROP_IH_FOLDER_PATH_DEFAULT);
    mem_servers_ = SeparateValue(p.GetProperty(PROP_MEM_SERVERS, PROP_MEM_SERVERS_DEFAULT), ',');
    auto vec_of_string_t = SeparateValue(p.GetProperty(PROP_MEM_PER_SERVER_GIB, PROP_MEM_PER_SERVER_GIB_DEFAULT), ',');
    for (auto &s : vec_of_string_t) per_server_chunk_quota_.push_back(std::stoull(s) * GIB_TO_CHUNK);
    for (size_t i = 0; i < mem_servers_.size(); i++) per_server_allocated_chunk_idx_.push_back(0);

    struct stat info;
    if (::stat(folder_path_.c_str(), &info) != 0 || !S_ISDIR(info.st_mode)) {
        if (::mkdir(folder_path_.c_str(), 0777) != 0) {
            LOG(ERROR) << "Can't make directory, error: " << strerror(errno);
            throw Exception("Can't make directory");
        }
    }

    nexus_->register_req_func(FETCH_IMAGE, FetchImageHandler);

    // start n threads to handle append entry RPCs
    const int n_th = std::stoi(p.GetProperty(PROP_THREADCOUNT, PROP_THREADCOUNT_DEFAULT));
    for (int i = 0; i < n_th; i++) {
        server_threads_.emplace_back(std::move(std::thread(ImageServer::fetch_server_func, p, i)));
    }

    while (run_) {
        ;  // do something, print server statistics etc
        sleep(1);
    }
}

void ImageServer::Finalize() {
    for (auto &t : server_threads_) t.join();
    delete nexus_;
}

void ImageServer::fetch_server_func(const Properties &p, int thread_id) {
    const uint8_t phy_port = std::stoi(p.GetProperty("erpc.phy_port", "0"));

    if (!rpc_)
        rpc_ =
            new erpc::Rpc<erpc::CTransport>(nexus_, nullptr, IH_SVR_RPCID_OFFSET + thread_id, svr_sm_handler, phy_port);
    rpc_use_cnt_.fetch_add(1);

    const uint64_t msg_size = std::stoull(p.GetProperty(PROP_IH_MSG_SIZE, PROP_IH_MSG_SIZE_DEFAULT));
    rpc_->set_pre_resp_msgbuf_size(msg_size);

    while (run_) {
        rpc_->run_event_loop(1000);
    }

    if (rpc_use_cnt_.fetch_sub(1) == 1) delete rpc_;
}

void ImageServer::Put(std::string &image_name) {
    if (image_map_.find(image_name) != image_map_.end()) {
        return;
    }
}

void ImageServer::Get(std::string &image_name, std::vector<std::pair<uint64_t, uint64_t>> &chunks) {}

void ImageServer::FetchImageHandler(erpc::ReqHandle *req_handle, void *context) {
    auto *req = req_handle->get_req_msgbuf();
    auto &resp = req_handle->pre_resp_msgbuf_;

    // extract image name string from req
    std::string image_name = std::string(req->buf_, req->buf_ + req->get_data_size());

    rpc_->resize_msg_buffer(&resp, 14);
    rpc_->enqueue_response(req_handle, &resp);
}

}  // namespace imageharbour