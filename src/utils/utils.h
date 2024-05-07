#pragma once

#include <string.h>

#include <algorithm>
#include <exception>
#include <string>

#include "../rpc/common.h"

namespace imageharbour {

class Exception : public std::exception {
   public:
    Exception(const std::string &message) : message_(message) {}
    const char *what() const noexcept { return message_.c_str(); }

   private:
    std::string message_;
};

inline bool StrToBool(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    if (str == "true" || str == "1") {
        return true;
    } else if (str == "false" || str == "0") {
        return false;
    } else {
        throw Exception("Invalid bool string: " + str);
    }
}

inline std::string Trim(const std::string &str) {
    auto front = std::find_if_not(str.begin(), str.end(), [](int c) { return std::isspace(c); });
    return std::string(front, std::find_if_not(str.rbegin(), std::string::const_reverse_iterator(front), [](int c) {
                                  return std::isspace(c);
                              }).base());
}

inline bool StrStartWith(const char *str, const char *pre) { return strncmp(str, pre, strlen(pre)) == 0; }

inline size_t SerializeChunkInfo(std::vector<std::pair<uint64_t, uint64_t>> &chunks, const char *digest,
                                 const uint64_t image_size, uint8_t *buf) {
    size_t offset = 0;
    *reinterpret_cast<uint64_t *>(buf) = chunks.size();
    offset += sizeof(uint64_t);
    for (auto &chunk : chunks) {
        *reinterpret_cast<uint64_t *>(buf + offset) = chunk.first;
        offset += sizeof(uint64_t);
        *reinterpret_cast<uint64_t *>(buf + offset) = chunk.second;
        offset += sizeof(uint64_t);
    }
    memcpy(buf + offset, digest, SHA256_DIGEST_SIZE);
    offset += SHA256_DIGEST_SIZE;
    *reinterpret_cast<uint64_t *>(buf + offset) = image_size;
    offset += sizeof(uint64_t);
    return offset;
}

inline size_t DeSerializeChunkInfo(std::vector<std::pair<uint64_t, uint64_t>> &chunks, char *digest,
                                   uint64_t &image_size, const uint8_t *buf) {
    size_t offset = 0;
    uint64_t num_chunks = *reinterpret_cast<const uint64_t *>(buf);
    offset += sizeof(uint64_t);
    for (uint64_t i = 0; i < num_chunks; ++i) {
        uint64_t start = *reinterpret_cast<const uint64_t *>(buf + offset);
        offset += sizeof(uint64_t);
        uint64_t end = *reinterpret_cast<const uint64_t *>(buf + offset);
        offset += sizeof(uint64_t);
        chunks.emplace_back(start, end);
    }
    memcpy(digest, buf + offset, SHA256_DIGEST_SIZE);
    offset += SHA256_DIGEST_SIZE;
    image_size = *reinterpret_cast<const uint64_t *>(buf + offset);
    offset += sizeof(uint64_t);
    return offset;
}
}  // namespace imageharbour