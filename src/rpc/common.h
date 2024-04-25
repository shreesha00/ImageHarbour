#pragma once

#include <string>

#include "log_entry.h"

namespace imageharbour {

const std::string PROP_IH_SVR_URI = "image_harbour.server_uri";
const std::string PROP_IH_SVR_URI_DEFAULT = "localhost:31850";

const std::string PROP_IH_CLI_URI = "image_harbour.client_uri";
const std::string PROP_IH_CLI_URI_DEFAULT = "localhost:31851";

const std::string PROP_IH_MSG_SIZE = "image_harbour.msg_size";
const std::string PROP_IH_MSG_SIZE_DEFAULT = "8192";

const std::string PROP_IH_FOLDER_PATH = "image_harbour.folder_path";
const std::string PROP_IH_FOLDER_PATH_DEFAULT = "/data/";

const uint8_t IH_CLI_RPCID_OFFSET = 0;
const uint8_t IH_SVR_RPCID_OFFSET = 64;

/* Image Server Interfaces */
const uint8_t FETCH_IMAGE = 1;

enum Status {
    OK = 0,
    ERROR = 1,
    NOENT = 2,
    NOFILE = 3,
};

}  // namespace imageharbour