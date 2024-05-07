#include "../rpc/common.h"
#include "../utils/properties.h"
#include "image_harbour_cli.h"

int main(int argc, const char *argv[]) {
    using namespace imageharbour;

    Properties prop;
    ParseCommandLine(argc, argv, prop);

    ImageHarbourClient cli(prop);

    cli.InitializeConn(prop, prop.GetProperty(PROP_IH_SVR_URI, PROP_IH_SVR_URI_DEFAULT), nullptr);

    std::vector<std::pair<uint64_t, uint64_t>> metadata;
    char digest[SHA256_DIGEST_SIZE];
    uint64_t size = 0;
    std::string filename = "/mydata/hello-world.tar";
    cli.FetchImageMetadata("hello-world", metadata, digest, size);
    cli.FetchImage(metadata, size, filename);

    std::cout << "chunk info:" << std::endl;
    for (const auto &m : metadata) {
        std::cout << m.first << " " << m.second << std::endl;
    }

    std::cout << "digest: " << digest << std::endl;
    std::cout << "size: " << size << std::endl;

    return 0;
}