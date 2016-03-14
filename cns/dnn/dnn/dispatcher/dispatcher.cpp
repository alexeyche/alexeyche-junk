#include "dispatcher.h"


namespace NDnn {

	TDispatcher::TDispatcher(ui32 port)
		: Server(port)
		, InputDataIsReadyVar(false)
	{
		Server
			.AddCallback(
				"POST", "api/input",
				[&](const THttpRequest& req, TResponseBuilder& resp) {
					NDnnProto::TTimeSeries ts;
					ENSURE(ts.ParseFromString(req.Body), "Failed to deserialize input TTimeSeries from http request");
					
					TUniqueLock lock(InputDataMutex);
					InputData.Deserialize(ts);
					InputDataIsReady.notify_all();
					InputDataIsReadyVar = true;
					InputDataIdx.resize(InputData.Dim());
					resp.Good();
				}
			);
	}
		
	TDispatcher::TDispatcher(const TDispatcher& other)
		: Server(other.GetPort()) 
	{
		(*this) = other;
	}

	TDispatcher& TDispatcher::operator =(const TDispatcher& other) {
		if (this != &other) {
			Server = other.Server;
			InputData = other.InputData;
			InputDataIsReadyVar = other.InputDataIsReadyVar;
		}
		return *this;
	}


	void TDispatcher::SetPort(ui32 port) {
		Server.SetPort(port);
	}
	
	const ui32& TDispatcher::GetPort() const {
		return Server.GetPort();
	}

	double TDispatcher::GetNeuronInput(ui32 layerId, ui32 neuronId) {
		while (!InputDataIsReadyVar) {
			L_DEBUG << "Waiting for data";
			TUniqueLock lock(InputDataMutex);
			InputDataIsReady.wait(lock);
		}
		if (layerId != 0) {
			return 0.0;
		}
		ENSURE((neuronId < InputData.Data.size()) && (InputDataIdx[neuronId] < InputData.Data[neuronId].Values.size()), "Id out of range");
		return InputData.Data[neuronId].Values[InputDataIdx[neuronId]++];
	}

	void TDispatcher::MainLoop() {
		L_DEBUG << "Entering dispatcher main loop on port " << Server.GetPort();
		Server.MainLoop();
	}

} // namspace NDnn