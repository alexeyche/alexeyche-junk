#include "dispatcher.h"


namespace NDnn {

	TDispatcher::TDispatcher(ui32 port)
		: Server(port)
	{
		Server
			.AddCallback(
				"GET", "api/input",
				[&](const THttpRequest& req, TResponseBuilder& resp) {
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
		Server.MainLoop();
	}

} // namspace NDnn