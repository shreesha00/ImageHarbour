#include "transport.h"

namespace imageharbour {

bool RPCTransport::run_ = true;

void RPCTransport::Stop() { run_ = false; }

RPCTransport::RPCTransport() {}

}  // namespace imageharbour