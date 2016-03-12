#pragma once

#include <dnn/base/base.h>
#include <dnn/util/server/server.h>


namespace NDnn {
	
	class TDispatcher {
	public:
		TDispatcher(ui32 port);
		
		void SetPort(ui32 port);

		void GetNeuronInput(ui32 layerId, ui32 neuronId);

		void MainLoop();
	private:
		TServer Server;
	};


} // namspace NDnn