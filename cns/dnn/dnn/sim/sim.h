#include "layer.h"

#include <dnn/base/base.h>

#include <dnn/util/spinning_barrier.h>
#include <dnn/util/log/log.h>
#include <dnn/util/thread.h>
#include <dnn/util/tuple.h>

#include <dnn/neuron/integrate_and_fire.h>

#include <utility>

namespace NDnn {

	struct TSimConfiguration {
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
				SimLayer(layer, perLayerJobs[layerId].Size, threads, barrier);	
			});

			for(auto& t: threads) {
				t.join();
			}
		}
		
		void SerializeToTextStream(std::ostream& ostr) {
			TSerialStreamProtoTxt str(ostr);
			ForEach(Layers, [&](auto& layer) {
				for (auto& n: layer) {
					n.SerialProcess(str);
				}
			});
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
		void SimLayer(L& layer, ui32 jobs, TVector<std::thread>& threads, TSpinningBarrier& barrier) {
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
		TSimConfiguration Conf;

		std::tuple<T ...> Layers;
	};


	template <typename ... T>
	auto BuildSim() {
		return TSim<T...>();
	}


} // namespace NDnn