#define TBB_PREVIEW_CONCURRENT_LRU_CACHE 1

#include <tbb/concurrent_lru_cache.h>

#include <iostream>

using ValueType = std::vector<std::pair<uint64_t, uint64_t>>;

ValueType generateEmptyValue(std::string key) { return {}; }

int main() {
    using CacheType = tbb::concurrent_lru_cache<std::string, ValueType>;
    CacheType cache(generateEmptyValue, 3);

    {
        auto h = cache["key1"];
        if (h) {
            h.value() = {{1, 2}, {3, 4}};
        }
    }

    // Accessing data from cache
    auto result = cache["key1"];
    if (result) {
        std::cout << "Found key1: ";
        for (const auto& pair : result.value()) std::cout << "(" << pair.first << ", " << pair.second << ") ";
        std::cout << std::endl;
    } else {
        std::cout << "Key1 not found!" << std::endl;
    }

    return 0;
}