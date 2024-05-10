#include "image_server.h"

#include "sys/mman.h"

namespace imageharbour {

std::string ImageServer::folder_path_ = "";
std::unordered_map<std::string, std::tuple<std::vector<std::pair<uint64_t, uint64_t>>, std::string, uint64_t>>
    ImageServer::image_cache_;
std::shared_mutex ImageServer::image_cache_mutex_;
std::vector<uint64_t> ImageServer::per_server_allocated_chunk_idx_;
std::vector<std::unique_ptr<std::mutex>> ImageServer::per_server_mutex_;
std::vector<uint64_t> ImageServer::per_server_chunk_quota_;
std::vector<std::string> ImageServer::mem_servers_;
char *ImageServer::scratch_pad_ = nullptr;
std::vector<std::vector<infinity::queues::QueuePair *>> ImageServer::per_thread_qps_;
infinity::core::Context *ImageServer::context_ = nullptr;
infinity::queues::QueuePairFactory *ImageServer::qp_factory_ = nullptr;

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
    for (size_t i = 0; i < mem_servers_.size(); i++) {
        per_server_mutex_.emplace_back(std::make_unique<std::mutex>());
        per_server_allocated_chunk_idx_.emplace_back(0);
    }
    scratch_pad_ = new char[SCRATCH_PAD_SIZE];

    struct stat info;
    if (::stat(folder_path_.c_str(), &info) != 0 || !S_ISDIR(info.st_mode)) {
        if (::mkdir(folder_path_.c_str(), 0777) != 0) {
            LOG(ERROR) << "Can't make directory, error: " << strerror(errno);
            throw Exception("Can't make directory");
        }
    }

    // infinity setup
    context_ = new infinity::core::Context();
    qp_factory_ = new infinity::queues::QueuePairFactory(context_);

    // connect each thread through individual qps to all remote memory servers
    per_thread_qps_.reserve(std::stoi(p.GetProperty(PROP_THREADCOUNT, PROP_THREADCOUNT_DEFAULT)));
    for (uint64_t i = 0; i < std::stoi(p.GetProperty(PROP_THREADCOUNT, PROP_THREADCOUNT_DEFAULT)); i++) {
        per_thread_qps_.emplace_back();
        for (uint64_t j = 0; j < mem_servers_.size(); j++) {
            LOG(INFO) << "connecting to remote server: " << mem_servers_[j];
            per_thread_qps_[i].push_back(qp_factory_->connectToRemoteHost(mem_servers_[j].c_str(), MEM_SERVER_PORT));
        }
    }

    for (auto &qps : per_thread_qps_) {
        for (auto &qp : qps) {
            infinity::memory::RegionToken *remote_buffer_token =
                static_cast<infinity::memory::RegionToken *>(qp->getUserData());
            LOG(INFO) << "remote buffer token: " << remote_buffer_token;
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

    // send terminate signal to memory servers, for buffer deregistration on the server side
    for (uint64_t i = 0; i < mem_servers_.size(); i++) {
        infinity::memory::Buffer *terminate = new infinity::memory::Buffer(context_, 128 * sizeof(char));
        infinity::requests::RequestToken requestToken(context_);

        per_thread_qps_[0][i]->send(terminate, &requestToken);
        requestToken.waitUntilCompleted();

        delete terminate;
    }

    // delete qps
    for (uint64_t i = 0; i < server_threads_.size(); i++) {
        for (uint64_t j = 0; j < mem_servers_.size(); j++) {
            delete per_thread_qps_[i][j];
        }
    }

    delete qp_factory_;
    delete context_;
    delete nexus_;
    delete scratch_pad_;
}

void ImageServer::fetch_server_func(const Properties &p, int thread_id) {
    ServerContext c;
    c.thread_id = thread_id;
    const uint8_t phy_port = std::stoi(p.GetProperty("erpc.phy_port", "0"));

    if (!rpc_)
        rpc_ = new erpc::Rpc<erpc::CTransport>(nexus_, static_cast<void *>(&c), IH_SVR_RPCID_OFFSET + thread_id,
                                               svr_sm_handler, phy_port);
    rpc_use_cnt_.fetch_add(1);

    const uint64_t msg_size = std::stoull(p.GetProperty(PROP_IH_MSG_SIZE, PROP_IH_MSG_SIZE_DEFAULT));
    rpc_->set_pre_resp_msgbuf_size(msg_size);

    while (run_) {
        rpc_->run_event_loop(1000);
    }

    if (rpc_use_cnt_.fetch_sub(1) == 1) delete rpc_;
}

uint64_t ImageServer::GetFreeChunks(uint64_t node, uint64_t num_chunks) {
    std::unique_lock<std::mutex> lock(*per_server_mutex_[node]);
    if (per_server_allocated_chunk_idx_[node] + num_chunks - 1 >= per_server_chunk_quota_[node]) {
        LOG(ERROR) << "No more free chunks";
        return -1;
    }
    per_server_allocated_chunk_idx_[node] += num_chunks;
    return per_server_allocated_chunk_idx_[node] - num_chunks;
}

void ImageServer::Get(std::string &image_name, std::vector<std::pair<uint64_t, uint64_t>> &chunks, char *digest,
                      uint64_t &image_size, int thread_id) {
    {
        std::shared_lock<std::shared_mutex> read_lock(image_cache_mutex_);
        if (image_cache_.find(image_name) != image_cache_.end()) {
            chunks = std::get<0>(image_cache_[image_name]);
            memcpy(digest, std::get<1>(image_cache_[image_name]).c_str(), SHA256_DIGEST_SIZE);
            image_size = std::get<2>(image_cache_[image_name]);
            return;
        }
    }
    {
        std::unique_lock<std::shared_mutex> write_lock(image_cache_mutex_);
        if (image_cache_.find(image_name) != image_cache_.end()) {
            chunks = std::get<0>(image_cache_[image_name]);
            memcpy(digest, std::get<1>(image_cache_[image_name]).c_str(), SHA256_DIGEST_SIZE);
            image_size = std::get<2>(image_cache_[image_name]);
            return;
        }

        // fetch and store image locally
        auto string_wo = image_name;
        string_wo.erase(std::remove(string_wo.begin(), string_wo.end(), '/'), string_wo.end());
        auto fetch_image_cmd = "docker pull " + image_name;
        auto store_image_cmd = "docker save " + image_name + " -o " + folder_path_ + string_wo + ".tar";
        std::ignore = system(fetch_image_cmd.c_str());
        std::ignore = system(store_image_cmd.c_str());

        // get sha256 digest of image
        auto sha256_cmd = "docker inspect --format='{{index .RepoDigests 0}}' " + image_name;
        FILE *fp = popen(sha256_cmd.c_str(), "r");
        if (fp == nullptr) {
            LOG(ERROR) << "Can't get sha256 digest";
            return;
        }
        if (fgets(digest, SHA256_DIGEST_SIZE, fp) == nullptr) {
            LOG(ERROR) << "Can't get sha256 digest";
            return;
        }
        pclose(fp);

        // read image file
        int fd = open((folder_path_ + string_wo + ".tar").c_str(), O_RDONLY);
        struct stat info;
        if (fstat(fd, &info) < 0) {
            LOG(ERROR) << "Can't stat image file";
            close(fd);
            return;
        }

        // pick a node to store image on
        std::hash<std::string> hasher;
        uint64_t memory_server = hasher(image_name) % mem_servers_.size();
        LOG(INFO) << "thread_id: " << thread_id << ", storing image on node: " << memory_server;

        // store individual chunks
        // TODO: currently assumes all chunks can come from a single node, might not be true.
        uint64_t chunk_size_bytes = CACHE_GRANULARITY_MIB * MIB;
        uint64_t num_chunks = (info.st_size + chunk_size_bytes - 1) / chunk_size_bytes;
        uint64_t remote_start_chunk = GetFreeChunks(memory_server, num_chunks);

        uint64_t curr_chunk = 0;
        infinity::memory::Buffer *buffer = new infinity::memory::Buffer(context_, chunk_size_bytes);
        infinity::requests::RequestToken request_token(context_);
        while (curr_chunk < num_chunks) {
            uint64_t read_size = std::min(SCRATCH_PAD_SIZE, info.st_size - curr_chunk * chunk_size_bytes);
            if (pread(fd, scratch_pad_, read_size, curr_chunk * chunk_size_bytes) != read_size) {
                LOG(ERROR) << "Reading less bytes than expected";
                close(fd);
                return;
            }
            for (uint64_t i = 0; i < (read_size + chunk_size_bytes - 1) / chunk_size_bytes; i++) {
                uint64_t rem_size = std::min(chunk_size_bytes, read_size - i * chunk_size_bytes);
                memcpy(buffer->getData(), scratch_pad_ + i * chunk_size_bytes, rem_size);
                per_thread_qps_[thread_id][memory_server]->write(
                    buffer, 0,
                    static_cast<infinity::memory::RegionToken *>(
                        per_thread_qps_[thread_id][memory_server]->getUserData()),
                    (remote_start_chunk + curr_chunk) * chunk_size_bytes, rem_size, &request_token);

                request_token.waitUntilCompleted();
                curr_chunk++;
            }
        }

        std::vector<std::pair<uint64_t, uint64_t>> chunk_list;
        for (uint64_t i = 0; i < num_chunks; i++) {
            chunk_list.emplace_back(remote_start_chunk + i, memory_server);
        }

        image_cache_[image_name] = {std::move(chunk_list), std::string(digest), info.st_size};
        chunks = std::get<0>(image_cache_[image_name]);
        image_size = std::get<2>(image_cache_[image_name]);

        close(fd);
        delete buffer;
        return;
    }
}

void ImageServer::FetchImageHandler(erpc::ReqHandle *req_handle, void *context) {
    auto *req = req_handle->get_req_msgbuf();
    auto &resp = req_handle->pre_resp_msgbuf_;

    // extract image name string from req
    std::string image_name = std::string(req->buf_, req->buf_ + req->get_data_size());

    char sha256_digest[SHA256_DIGEST_SIZE];
    std::vector<std::pair<uint64_t, uint64_t>> chunks;
    uint64_t image_size = 0;
    Get(image_name, chunks, sha256_digest, image_size, static_cast<ServerContext *>(context)->thread_id);
    size_t len = SerializeChunkInfo(chunks, sha256_digest, image_size, resp.buf_);

    rpc_->resize_msg_buffer(&resp, len);
    rpc_->enqueue_response(req_handle, &resp);
}

}  // namespace imageharbour