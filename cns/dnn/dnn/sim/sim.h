#include "layer.h"

#include <dnn/base/base.h>

#include <dnn/util/spinning_barrier.h>
#include <dnn/util/log/log.h>
#include <dnn/util/thread.h>
#include <dnn/util/tuple.h>



namespace NDnn {

	struct TSimConf {
		ui32 Jobs = 4;
		double Duration = 1000;
	};

	template <typename ... T>
	class TSim {
	public:
		using TSelf = TSim<T...>;

		TSim() {}

		ui32 NeuronsNumber() {
			ui32 acc = 0;
			ForEach(Layers, [&](const auto& t) {
				acc += t.Size();
			});
			return acc;
		}
		
		void Run() {
			L_DEBUG << "Going to run simulation for " << Conf.Duration << " ms in " << Conf.Jobs << " jobs";
			TVector<TIndexSlice> slices = DispatchOnThreads(NeuronsNumber(), Conf.Jobs);
			TVector<std::thread> threads;
			
			TSpinningBarrier barrier(Conf.Jobs);
			ui32 layerId = 0;
			for(const auto& slice: slices) {
				threads.emplace_back(
					TSelf::RunWorker,
					std::ref(*this),
					layerId,
					slice.From,
					slice.To,
					std::ref(barrier)
				);
			}

			for(auto& t: threads) {
				t.join();
			}

		}

	private:
		void RunWorkerRoutine(ui32 layerId, ui32 idxFrom, ui32 idxTo, TSpinningBarrier& barrier) {
			TTime t(1.0);
			
			auto layer = std::get<0>(Layers);
			for(ui32 neuronId=idxFrom; neuronId<idxTo; ++neuronId) {
				layer[neuronId].Reset();
			}

			for(ui32 neuronId=idxFrom; neuronId<idxTo; ++neuronId) {
				layer[neuronId].CalculateDynamics(t, 0, 0);
			}
		}



		static void RunWorker(TSelf& self, ui32 layerId, ui32 idxFrom, ui32 idxTo, TSpinningBarrier& barrier) {
			try {
				self.RunWorkerRoutine(layerId, idxFrom, idxTo, barrier);
			} catch (const TDnnException& e) {
				L_DEBUG << "Got error in layer " << layerId << ", neurons " << idxFrom << ":" << idxTo << " thread: " << e.what();
				barrier.Fail();
			} catch (const TDnnInterrupt& e) {
				// pass
			}
		}
	private:
		TSimConf Conf;

		std::tuple<T ...> Layers;
	};



} // namespace NDnn