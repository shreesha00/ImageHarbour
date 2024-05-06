#include "../rpc/common.h"
#include "../utils/properties.h"
#include "image_harbour_cli.h"

int main(int argc, const char *argv[]) {
    using namespace imageharbour;

    Properties prop;
    ParseCommandLine(argc, argv, prop);

    ImageHarbourClient cli;

    cli.InitializeConn(prop, prop.GetProperty(PROP_IH_SVR_URI, PROP_IH_SVR_URI_DEFAULT), nullptr);

    std::vector<std::pair<uint64_t, uint64_t>> metadata;
    cli.FetchImageMetadata("debian", metadata);

    for (const auto &m : metadata) {
        std::cout << m.first << " " << m.second << std::endl;
    }

    return 0;
}