#pragma once

#include <dnn/base/base.h>
#include <dnn/util/ptr.h>

#include <random>
#include <chrono>

namespace NDnn {
	
	class TRandEngine {
	public:
		TRandEngine(int seed)
			: Generator(seed >= 0 ? seed : std::chrono::system_clock::now().time_since_epoch().count())
			, Unif(0.0,1.0)
		{}
	
		double GetUnif() {
			return Unif(Generator);
		}	

	private:
		std::uniform_real_distribution<double> Unif;
		std::mt19937 Generator;
	};

} // namespace NDnn