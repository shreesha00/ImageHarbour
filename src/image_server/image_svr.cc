#include "image_server.h"
#include "signal.h"
using namespace imageharbour;

void sigint_handler(int _signum) { RPCTransport::Stop(); }

int main(int argc, const char *argv[]) {
    signal(SIGINT, sigint_handler);

    Properties prop;
    ParseCommandLine(argc, argv, prop);

    ImageServer image_svr;
    image_svr.Initialize(prop);
    image_svr.Finalize();

    return 0;
}