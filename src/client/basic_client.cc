#include "../rpc/common.h"
#include "../utils/properties.h"
#include "image_harbour_cli.h"

int main(int argc, const char *argv[]) {
    using namespace imageharbour;

    Properties prop;
    ParseCommandLine(argc, argv, prop);

    ImageHarbourClient cli(prop);

    cli.InitializeConn(prop, prop.GetProperty(PROP_IH_SVR_URI, PROP_IH_SVR_URI_DEFAULT), nullptr);

    cli.FetchImage("debian", "/data/debian.tar");
    return 0;
}