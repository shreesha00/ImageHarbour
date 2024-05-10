#include <hdr/hdr_histogram.h>

#include <chrono>

#include "../rpc/common.h"
#include "../utils/properties.h"
#include "image_harbour_cli.h"

int main(int argc, const char* argv[]) {
    using namespace imageharbour;
    hdr_histogram *fetch_time_hist, *load_time_hist, *total_time_hist;
    hdr_init(1, INT64_C(3600000000), 3, &fetch_time_hist);
    hdr_init(1, INT64_C(3600000000), 3, &load_time_hist);
    hdr_init(1, INT64_C(3600000000), 3, &total_time_hist);

    Properties prop;
    ParseCommandLine(argc, argv, prop);

    uint64_t time_secs = std::stoull(prop.GetProperty("time", "120"));
    std::string filename = prop.GetProperty("filename", "debian.tar");
    std::string filepath = prop.GetProperty("filepath", "/data/debian.tar");

    ImageHarbourClient cli(prop);

    cli.InitializeConn(prop, prop.GetProperty(PROP_IH_SVR_URI, PROP_IH_SVR_URI_DEFAULT), nullptr);

    std::string load_cmd = "docker load < " + filepath;
    std::string prune_cmd = "docker system prune -a -f";
    auto start = std::chrono::high_resolution_clock::now();
    while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start).count() <
           time_secs) {
        auto start_fetch = std::chrono::high_resolution_clock::now();
        cli.FetchImage(filename, filepath);
        hdr_record_value(fetch_time_hist, std::chrono::duration_cast<std::chrono::microseconds>(
                                              std::chrono::high_resolution_clock::now() - start_fetch)
                                              .count());

        auto start_load = std::chrono::high_resolution_clock::now();
        std::ignore = system(load_cmd.c_str());
        hdr_record_value(load_time_hist, std::chrono::duration_cast<std::chrono::microseconds>(
                                             std::chrono::high_resolution_clock::now() - start_load)
                                             .count());
        hdr_record_value(total_time_hist, std::chrono::duration_cast<std::chrono::microseconds>(
                                              std::chrono::high_resolution_clock::now() - start_fetch)
                                              .count());
        std::ignore = system(prune_cmd.c_str());
    }

    LOG(INFO) << "fetch time histogram:" << std::endl;
    hdr_percentiles_print(fetch_time_hist, stdout, 5, 1.0, CLASSIC);

    LOG(INFO) << "load time histogram:" << std::endl;
    hdr_percentiles_print(load_time_hist, stdout, 5, 1.0, CLASSIC);

    LOG(INFO) << "total time histogram:" << std::endl;
    hdr_percentiles_print(total_time_hist, stdout, 5, 1.0, CLASSIC);
    return 0;
}