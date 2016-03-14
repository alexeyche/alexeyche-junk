#pragma once

#include <dnn/connection/connection.h>
#include <dnn/protos/config.pb.h>

namespace NDnn {

	TConnectionPtr BuildConnection(const NDnnProto::TConnection& conn);

} // namespace NDnn