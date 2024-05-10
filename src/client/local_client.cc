#include <hdr/hdr_histogram.h>

#include <chrono>
#include <string>
#include <iostream>

int main(int argc, const char* argv[]) {
    hdr_histogram *fetch_time_hist, *load_time_hist, *total_time_hist;
    hdr_init(1, INT64_C(3600000000), 3, &fetch_time_hist);
    hdr_init(1, INT64_C(3600000000), 3, &load_time_hist);
    hdr_init(1, INT64_C(3600000000), 3, &total_time_hist);

    // std::string pull_cmd = "docker pull node5:5000/pytorch/pytorch:latest";
    // std::string pull_cmd = "docker pull node5:5000/hello-world:latest";
    // std::string pull_cmd = "docker pull node5:5000/postgres:15";
    // std::string pull_cmd = "docker pull node5:5000/debian";
    std::string pull_cmd = "docker pull node5:5000/alpine";
    std::string prune_cmd = "docker system prune -af";
    std::string drop_cache = "sudo sh -c 'echo 3 > /proc/sys/vm/drop_caches'";
    auto start = std::chrono::high_resolution_clock::now();
    while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start).count() <
           180) {
        (void) system(prune_cmd.c_str());
        (void) system(drop_cache.c_str());
        auto start_fetch = std::chrono::high_resolution_clock::now();
        (void) system(pull_cmd.c_str());
        hdr_record_value(fetch_time_hist, std::chrono::duration_cast<std::chrono::microseconds>(
                                              std::chrono::high_resolution_clock::now() - start_fetch)
                                              .count());
    }

    hdr_percentiles_print(fetch_time_hist, stdout, 5, 1.0, CLASSIC);
    return 0;
}