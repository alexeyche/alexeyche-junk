#include "layer.h"

#include <dnn/base/base.h>

#include <dnn/util/spinning_barrier.h>
#include <dnn/util/log/log.h>
#include <dnn/util/thread.h>
#include <dnn/util/tuple.h>

#include <dnn/neuron/integrate_and_fire.h>

namespace NDnn {

	struct TSimConf {
		ui32 Jobs = 4;
		double Duration = 1000;
		double Dt = 1.0;
	};

	template <typename ... T>
	class TSim {
	public:
		using TSelf = TSim<T...>;

		TSim() {
			ForEachEnumerate(Layers, [&](ui32 layerId, auto& l) {
				l.SetId(layerId);
			});
		}
		
		void Run() {
			L_DEBUG << "Going to run simulation for " << Conf.Duration << " ms in " << Conf.Jobs << " jobs";
			TVector<TIndexSlice> perLayerJobs = DispatchOnThreads(Conf.Jobs, std::tuple_size<decltype(Layers)>::value);
			
		 	TSpinningBarrier barrier(Conf.Jobs);
			TVector<std::thread> threads;

			ForEachEnumerate(Layers, [&](ui32 layerId, auto& layer) {
				if (Conf.Jobs == 1) {
					SimLayer(layer, perLayerJobs[layerId].Size, threads, barrier, /* sync = */ true);	
				} else {
					SimLayer(layer, perLayerJobs[layerId].Size, threads, barrier, /* sync = */ false);	
				}
				
			});

			for(auto& t: threads) {
				t.join();
			}
		}

	private:

		template <typename L>
		void RunWorkerRoutine(L& layer, ui32 idxFrom, ui32 idxTo, TSpinningBarrier& barrier) {
			TTime t(Conf.Dt);
			L_DEBUG << "Entering into simulation of layer " << layer.GetId() << " of neurons " << idxFrom << ":" << idxTo;
			for (ui32 neuronId=idxFrom; neuronId<idxTo; ++neuronId) {
				layer[neuronId].GetNeuron().Reset();	
			}


			for (; t < Conf.Duration; ++t) {
				for(ui32 neuronId=idxFrom; neuronId<idxTo; ++neuronId) {
					layer[neuronId].CalculateDynamicsInternal(t);
				}
			}
			

		}

		template <typename L>
		void SimLayer(L& layer, ui32 jobs, TVector<std::thread>& threads, TSpinningBarrier& barrier, bool sync) {
			if (sync) {
				TSelf::RunWorker<L>(*this, layer, 0, layer.Size(), barrier);
			} else {
				TVector<TIndexSlice> layerJobSlices = DispatchOnThreads(layer.Size(), jobs);
				for (const auto& slice: layerJobSlices) {
					threads.emplace_back(
						TSelf::RunWorker<L>,
						std::ref(*this),
						std::ref(layer),
						slice.From,
						slice.To,
						std::ref(barrier)
					);	
				}	
			}
			
			
		}

		template <typename L>
		static void RunWorker(TSelf& self, L& layer, ui32 idxFrom, ui32 idxTo, TSpinningBarrier& barrier) {
			try {
				self.RunWorkerRoutine(layer, idxFrom, idxTo, barrier);
			} catch (const TDnnException& e) {
				L_DEBUG << "Got error in layer " << layer.GetId() << ", neurons " << idxFrom << ":" << idxTo << ", thread: " << e.what();
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