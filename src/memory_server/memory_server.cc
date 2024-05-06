#include <infinity/core/Context.h>
#include <infinity/memory/Buffer.h>
#include <infinity/memory/RegionToken.h>
#include <infinity/queues/QueuePair.h>
#include <infinity/queues/QueuePairFactory.h>
#include <infinity/requests/RequestToken.h>

#include "../rpc/common.h"
#include "../utils/properties.h"
#include "../utils/utils.h"
#include "glog/logging.h"

using namespace imageharbour;
int main(int argc, const char *argv[]) {
    Properties prop;
    ParseCommandLine(argc, argv, prop);

    uint64_t server_id = std::stoull(prop.GetProperty("server_id", "0"));
    std::vector<std::string> mem_servers =
        SeparateValue(prop.GetProperty(PROP_MEM_SERVERS, PROP_MEM_SERVERS_DEFAULT), ',');
    uint64_t memory_region_size =
        std::stoull(
            SeparateValue(prop.GetProperty(PROP_MEM_PER_SERVER_GIB, PROP_MEM_PER_SERVER_GIB_DEFAULT), ',')[server_id]) *
        GIB;

    LOG(INFO) << "[mem_server_" << server_id << "]: starting with region size " << memory_region_size << " bytes";
    infinity::core::Context *context = new infinity::core::Context();
    infinity::queues::QueuePairFactory *qp_factory = new infinity::queues::QueuePairFactory(context);

    infinity::memory::Buffer *memory_buffer = new infinity::memory::Buffer(context, memory_region_size);
    infinity::memory::RegionToken *buffer_token = memory_buffer->createRegionToken();

    infinity::memory::Buffer *terminate_buffer = new infinity::memory::Buffer(context, 128 * sizeof(char));
    context->postReceiveBuffer(terminate_buffer);

    qp_factory->bindToPort(MEM_SERVER_PORT);

    infinity::core::receive_element_t receive_element;
    std::vector<infinity::queues::QueuePair *> connected_qps;
    while (!context->receive(&receive_element)) {
        connected_qps.emplace_back(
            qp_factory->acceptIncomingConnection(buffer_token, sizeof(infinity::memory::RegionToken)));
        LOG(INFO) << "[mem_server_" << server_id << "]: accepted new connection";
    }

    LOG(INFO) << "[mem_server_" << server_id << "]: terminating...";

    for (auto &qp : connected_qps) {
        delete qp;
    }
    delete terminate_buffer;
    delete memory_buffer;
    delete qp_factory;
    delete context;
}