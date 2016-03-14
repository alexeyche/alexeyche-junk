#include "builder.h"


#include <dnn/connection/stochastic.h>

namespace NDnn {

	TConnectionPtr BuildConnection(const NDnnProto::TConnection& conn, TRandEngine& rand) {
		TConnectionPtr out;
		if (conn.has_stochasticconst()) {
			ENSURE(!out, "Got duplicates of connection type in connection specification: " << conn.DebugString());
			out = MakeShared(new TStochastic());
			out->Deserialize(conn);
		}
		ENSURE(out, "Connection is not implemented for " << conn.DebugString());
		out->SetRandEngine(rand);
		return out;
	}


} // namespace NDnn