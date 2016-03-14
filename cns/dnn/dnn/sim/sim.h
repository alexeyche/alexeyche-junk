#pragma once

#include "layer.h"
#include "network.h"

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
			serial(Seed);
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
			ui32 accNeuronsSize = 0;
			ForEachEnumerate(Layers, [&](ui32 layerId, auto& l) {
				l.SetupSpaceInfo(layerId, accNeuronsSize);
				accNeuronsSize += l.Size();
			});
			Network.Init(accNeuronsSize);
			ForEach(Layers, [&](auto& l) {
				Network.AddLayer(l);
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
				const NDnnProto::TConfig& inputConfig = serial.GetMessage<NDnnProto::TConfig>();
				CreateConnections(inputConfig);
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

//======= PERFOMANCE MEASURE ======================================================================
		#ifdef PERF
			std::time_t start_time = std::time(nullptr);
			double sim_time = t.T;
		#endif
//======= END =====================================================================================

			for (; t < Conf.Duration; ++t) {
				for(ui32 neuronId=idxFrom; neuronId<idxTo; ++neuronId) {
					Dispatcher.GetNeuronInput(layer.GetId(), neuronId);

					layer[neuronId].CalculateDynamicsInternal(t);
					if (layer[neuronId].GetNeuron().Fired()) {
						Network.PropagateSpike(layer[neuronId], t.T);
						layer[neuronId].GetNeuron().MutFired() = false;
					}
				}
				barrier.Wait();

//======= PERFOMANCE MEASURE ======================================================================
		#ifdef PERF
				size_t cur_time = std::time(nullptr);
				if(cur_time - start_time>5) {
					L_DEBUG << "Sim, perf start: " << ((double)(t.t-sim_time)/1000.0)/((double)(cur_time - start_time));
					start_time = cur_time;
					sim_time = t.T;
				}
		#endif
//======= END =====================================================================================

			}

			barrier.Wait();
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

		void CreateConnections(const NDnnProto::TConfig& config) {
			TRandEngine rand(Conf.Seed);
			for (const auto& connection: config.connection()) {
				ForEach(Layers, [&](auto& leftLayer) {
					if (leftLayer.GetId() != connection.from()) {
						return;
					}
					ForEach(Layers, [&](auto& rightLayer) {
						if (rightLayer.GetId() != connection.to()) {
							return;
						}
						L_DEBUG << "Connecting layer " << leftLayer.GetId() << " to " << rightLayer.GetId();
						leftLayer.Connect(rightLayer, connection, rand);
					});
				});
			}
		}


	private:
		TSimConfiguration Conf;

		std::tuple<T ...> Layers;
		TDispatcher Dispatcher;
		TNetwork Network;
	};


	template <typename ... T>
	auto BuildSim(ui32 port) {
		return TSim<T...>(port);
	}


} // namespace NDnn