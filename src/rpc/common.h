#pragma once

#include <string>

namespace imageharbour {

const std::string PROP_IH_SVR_URI = "image_harbour.server_uri";
const std::string PROP_IH_SVR_URI_DEFAULT = "localhost:31850";

const std::string PROP_IH_CLI_URI = "image_harbour.client_uri";
const std::string PROP_IH_CLI_URI_DEFAULT = "localhost:31851";

const std::string PROP_IH_MSG_SIZE = "image_harbour.msg_size";
const std::string PROP_IH_MSG_SIZE_DEFAULT = "8192";

const std::string PROP_IH_FOLDER_PATH = "image_harbour.folder_path";
const std::string PROP_IH_FOLDER_PATH_DEFAULT = "/data/";

const std::string PROP_THREADCOUNT = "image_harbour.threadcount";
const std::string PROP_THREADCOUNT_DEFAULT = "1";

const std::string PROP_MEM_SERVERS = "image_harbour.memory_servers";
const std::string PROP_MEM_SERVERS_DEFAULT = "localhost:31860";

const std::string PROP_MEM_PER_SERVER_GIB = "image_harbour.memory_per_server_gib";
const std::string PROP_MEM_PER_SERVER_GIB_DEFAULT = "1"

    const uint8_t IH_CLI_RPCID_OFFSET = 0;
const uint8_t IH_SVR_RPCID_OFFSET = 64;

/* Image Server Interfaces */
const uint8_t FETCH_IMAGE = 1;

const size_t PAGE_SIZE = 4096;

enum Status {
    OK = 0,
    ERROR = 1,
    NOENT = 2,
    NOFILE = 3,
};

}  // namespace imageharbour