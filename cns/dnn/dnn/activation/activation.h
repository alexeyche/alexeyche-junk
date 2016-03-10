#pragma once

#include <dnn/util/serial.h>


namespace NDnn {


	template <typename TConstants>
	class TActivation: public ISerialStream {
	protected:
		TActivation() {}
		
		void SerialProcess(TSerialStream& serial) override final {
			serial(c); 
		}

		TConstants c;
	};


} // namespace NDnn


