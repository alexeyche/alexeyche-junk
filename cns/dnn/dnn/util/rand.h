#pragma once

#include <dnn/base/base.h>
#include <dnn/util/ptr.h>

#include <random>
#include <chrono>

namespace NDnn {

	class TRandEngine {
	public:
		TRandEngine(int seed)
			: Unif(0.0,1.0)
			, Norm(0.0, 1.0)
		{
			ui32 seedNum = seed >= 0 ? seed : std::chrono::system_clock::now().time_since_epoch().count();
			Generator = std::mt19937(seedNum);
			L_DEBUG << "Initialize rand engine with seed " << seedNum;
		}

		double GetUnif() {
			return Unif(Generator);
		}

		double GetNorm() {
			return Norm(Generator);
		}
		
 	private:
		std::uniform_real_distribution<double> Unif;
		std::normal_distribution<double> Norm;

		std::mt19937 Generator;
	};

} // namespace NDnn