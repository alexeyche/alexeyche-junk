#include "dispatcher.h"


namespace NDnn {

	TDispatcher::TDispatcher(ui32 port)
		: Server(port)
	{
		Server
			.AddCallback(
				"POST", "api/input",
				[&](const THttpRequest& req, TResponseBuilder& resp) {
					NDnnProto::TTimeSeries ts;
					ENSURE(ts.ParseFromString(req.Body), "Failed to deserialize input TTimeSeries from http request");
					InputData.Deserialize(ts);
					resp.Good();
				}
			);
	}
		
	void TDispatcher::SetPort(ui32 port) {
		Server.Init(port);
	}
		
	void TDispatcher::GetNeuronInput(ui32 layerId, ui32 neuronId) {
		
	}

	void TDispatcher::MainLoop() {
		L_DEBUG << "Entering dispatcher main loop on port " << Server.GetPort();
		Server.MainLoop();
	}

} // namspace NDnn