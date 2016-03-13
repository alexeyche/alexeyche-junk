#include "layer.h"

#include <dnn/base/base.h>

#include <dnn/dispatcher/dispatcher.h>

#include <dnn/util/spinning_barrier.h>
#include <dnn/util/log/log.h>
#include <dnn/util/thread.h>
#include <dnn/util/tuple.h>
#include <dnn/util/rand.h>

#include <dnn/neuron/integrate_and_fire.h>

#include <dnn/protos/config.pb.h>

#include <utility>

namespace NDnn {

	struct TSimConfiguration: public IProtoSerial<NDnnProto::TSimConfiguration> {
		void SerialProcess(TProtoSerial& serial) override {
			serial(Jobs);
			serial(Duration);
			serial(Dt);
			serial(Port);
		}

		ui32 Jobs = 4;
		double Duration = 1000;
		double Dt = 1.0;
		ui32 Port = 9090;
		int Seed = -1;
	};



	template <typename ... T>
	class TSim: public IProtoSerial<NDnnProto::TConfig> {
	public:
		using TSelf = TSim<T...>;
		using TParent = IProtoSerial<NDnnProto::TConfig>;

		TSim(ui32 port)
			: Dispatcher(port) 
		{
			ForEachEnumerate(Layers, [&](ui32 layerId, auto& l) {
				l.SetId(layerId);
			});
		}
		
		void Run() {
			L_DEBUG << "Going to run simulation for " << Conf.Duration << " ms in " << Conf.Jobs << " jobs";
			TVector<TIndexSlice> perLayerJobs = DispatchOnThreads(Conf.Jobs, LayersSize());
			
		 	TSpinningBarrier barrier(Conf.Jobs);
			TVector<std::thread> threads;

			ForEachEnumerate(Layers, [&](ui32 layerId, auto& layer) {
				SimLayer(layer, perLayerJobs[layerId].Size, threads, barrier);	
			});
			threads.emplace_back([&]() {
				Dispatcher.MainLoop();
			});
			for(auto& t: threads) {
				t.join();
			}
		}
		
		ui32 LayersSize() const {
			return std::tuple_size<decltype(Layers)>::value;
		}

		void SerialProcess(TProtoSerial& serial) {
			serial(Conf, NDnnProto::TConfig::kSimConfigurationFieldNumber);
			
			serial.DuplicateSingleRepeated(NDnnProto::TConfig::kLayerFieldNumber, LayersSize());
			ForEach(Layers, [&](auto& layer) {
				serial(layer, NDnnProto::TConfig::kLayerFieldNumber, /* newMessage = */ true);
			});
			if (serial.IsInput()) {
				Dispatcher.SetPort(Conf.Port);
			}
		}

	private:

		template <typename L>
		void RunWorkerRoutine(L& layer, ui32 idxFrom, ui32 idxTo, TSpinningBarrier& barrier) {
			TTime t(Conf.Dt);
			TRandEngine rand(Conf.Seed);

			L_DEBUG << "Entering into simulation of layer " << layer.GetId() << " of neurons " << idxFrom << ":" << idxTo;
			
			for (ui32 neuronId=idxFrom; neuronId<idxTo; ++neuronId) {
				layer[neuronId].SetRandEngine(rand);
				layer[neuronId].Prepare();
			}

			for (; t < Conf.Duration; ++t) {
				for(ui32 neuronId=idxFrom; neuronId<idxTo; ++neuronId) {
					Dispatcher.GetNeuronInput(layer.GetId(), neuronId);

					layer[neuronId].CalculateDynamicsInternal(t);
				}
				barrier.Wait();
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
		TDispatcher Dispatcher;
	};


	template <typename ... T>
	auto BuildSim(ui32 port) {
		return TSim<T...>(port);
	}


} // namespace NDnn