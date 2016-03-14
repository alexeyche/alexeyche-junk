#pragma once

#include <condition_variable>

#include <dnn/base/base.h>
#include <dnn/util/server/server.h>
#include <dnn/util/ts/time_series.h>


namespace NDnn {
	
	class TDispatcher {
	public:
		TDispatcher(ui32 port);
		
		TDispatcher(const TDispatcher& other);

		TDispatcher& operator =(const TDispatcher& other);

		void SetPort(ui32 port);
		
		const ui32& GetPort() const;

		double GetNeuronInput(ui32 layerId, ui32 neuronId);

		void MainLoop();

	private:
		std::condition_variable InputDataIsReady;
		bool InputDataIsReadyVar;
		std::mutex	InputDataMutex;

		TTimeSeries InputData;

		TServer Server;
		TVector<ui32> InputDataIdx;
	};


} // namspace NDnn