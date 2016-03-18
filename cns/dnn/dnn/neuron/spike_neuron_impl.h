#pragma once

#include <queue>
#include <atomic>

#include <dnn/util/act_vector.h>
#include <dnn/util/random.h>
#include <dnn/util/rand.h>
#include <dnn/util/optional.h>
#include <dnn/util/serial/meta_proto_serial.h>
#include <dnn/util/serial/util.h>
#include <dnn/protos/config.pb.h>
#include <dnn/protos/spike_neuron_impl.pb.h>



namespace NDnn {

	struct TNeuronSpaceInfo {
		ui32 LayerId;
		ui32 LocalId;
		ui32 GlobalId;
		ui32 ColumnSize;
		ui32 RowId;
		ui32 ColId;

		bool operator == (const TNeuronSpaceInfo& other) const {
			return GlobalId == other.GlobalId;
		}

		friend std::ostream& operator<<(std::ostream& str, const TNeuronSpaceInfo& self) {
            str << "Neuron(g-" << self.GlobalId << ":layer-" << self.LayerId << ":local-" << self.LocalId << ")";
            return str;
        }
	};

	struct TSpikeNeuronImplInnerState: public IProtoSerial<NDnnProto::TSpikeNeuronImplInnerState> {
		void SerialProcess(TProtoSerial& serial) {
			serial(SynapsesSize);
		}

		ui32 SynapsesSize = 0;
	};

	struct TSpikeNeuronConst: public IProtoSerial<NDnnProto::TSpikeNeuronConst> {
		void SerialProcess(TProtoSerial& serial) {
			serial(AxonDelay);
		}

		double AxonDelay = 0.0;
	};

	class TSpikeNeuronImplInner: public IProtoSerial<NDnnProto::TLayer> {
	public:
		void SerialProcess(TProtoSerial& serial) {
			serial(s, NDnnProto::TLayer::kSpikeNeuronImplInnerStateFieldNumber);
			serial(c, NDnnProto::TLayer::kSpikeNeuronConstFieldNumber);
		}

		const ui32& SynapsesSize() const {
			return s.SynapsesSize;
		}

		ui32& MutSynapsesSize() {
			return s.SynapsesSize;
		}

		const double& GetAxonDelay() const {
			return c.AxonDelay;
		}

	private:
		TSpikeNeuronImplInnerState s;
		TSpikeNeuronConst c;
	};


	struct TAsyncSpikeQueue {
		TAsyncSpikeQueue()
			: InputSpikesLock(ATOMIC_FLAG_INIT)
		{}
		
		TAsyncSpikeQueue(const TAsyncSpikeQueue& other)
			: InputSpikesLock(ATOMIC_FLAG_INIT) 
		{
			(*this) = other;
		}
		
		TAsyncSpikeQueue& operator = (const TAsyncSpikeQueue& other) {
			if (this != &other) {
				InputSpikes = other.InputSpikes;
				Info = other.Info;
			}
			return *this;
		}
		
		void EnqueueSpike(const TSynSpike&& sp) {
			while (InputSpikesLock.test_and_set(std::memory_order_acquire));
			InputSpikes.push(sp);
			InputSpikesLock.clear(std::memory_order_release);
		}
		
		TNeuronSpaceInfo Info;

		std::priority_queue<TSynSpike> InputSpikes;
		std::atomic_flag InputSpikesLock;
	};

	template <typename TNeuron, typename TConf>
	class TSpikeNeuronImpl: public IMetaProtoSerial {
	public:
		using TNeuronType = TNeuron; 
		using TSelf = TSpikeNeuronImpl<TNeuron, TConf>;
		
		void ReadInputSpikes(const TTime &t) {
			while (Queue.InputSpikesLock.test_and_set(std::memory_order_acquire)) {}
			while (!Queue.InputSpikes.empty()) {
		        const TSynSpike& sp = Queue.InputSpikes.top();
		        if(sp.T >= t.T) break;
		        auto& s = Synapses[sp.SynapseId];
		        s.MutFired() = true;
		    	s.PropagateSpike();

		        Queue.InputSpikes.pop();
		    }
		    Queue.InputSpikesLock.clear(std::memory_order_release);
		}

		void CalculateDynamicsInternal(const TTime& t) {
			ReadInputSpikes(t);

			double Iinput = 0.0;

			double Isyn = 0.0;
			auto synIdIt = Synapses.abegin();
		    while (synIdIt != Synapses.aend()) {
		    	auto& synapse = Synapses[synIdIt];
		    	double x = synapse.WeightedPotential();
		    	if(fabs(x) < 0.0001) {
		        	Synapses.SetInactive(synIdIt);
		        } else {
		        	Isyn += x;
		        	++synIdIt;
		        }
		    }
			
			Neuron.CalculateDynamics(t, Iinput, Isyn);

		    Neuron.MutSpikeProbability() = Activation.SpikeProbability(Neuron.Membrane());
			if(Neuron.SpikeProbability() > Rand->GetUnif()) {
		        Neuron.MutFired() = true;
		        Neuron.PostSpikeDynamics(t);
		    }

	   		for(auto syn_id_it = Synapses.abegin(); syn_id_it != Synapses.aend(); ++syn_id_it) {
	        	auto& s = Synapses[syn_id_it];
	        	s.CalculateDynamics(t);
	        	s.MutFired() = false;
	        }
		}

		TNeuron& GetNeuron() {
			return Neuron;
		}

		void SetRandEngine(TRandEngine& rand) {
			Rand.Set(rand);
			Neuron.SetRandEngine(rand);
		}

		void Prepare() {
			ENSURE(Rand, "Random engine is not set");

			Neuron.Reset();
		}

		void SetSpaceInfo(TNeuronSpaceInfo info) {
			SpaceInfo = info;
			Queue.Info = SpaceInfo;
		}

		template <typename TOtherNeuron>
		bool operator == (const TOtherNeuron& other) const {
			return SpaceInfo == other.GetSpaceInfo();
		}

		const TNeuronSpaceInfo& GetSpaceInfo() const {
			return SpaceInfo;
		}

		TAsyncSpikeQueue& GetMutAsyncSpikeQueue() {
			return Queue;
		}

		const ui32& GetGlobalId() const {
			return SpaceInfo.GlobalId;
		}

		const ui32& GetLocalId() const {
			return SpaceInfo.LocalId;
		}

		void AddSynapse(typename TConf::TSynapse&& syn) {
			Synapses.emplace_back(std::forward<typename TConf::TSynapse>(syn));
			Inner.MutSynapsesSize()++;
		}

		void SerialProcess(TMetaProtoSerial& serial) override final {
			serial(Neuron);
			serial(Activation);
			serial(Inner);
			if (serial.IsInput()) {
				Synapses.resize(Inner.SynapsesSize());
				if (Inner.SynapsesSize() == 0) {
					const NDnnProto::TLayer& layerSpec = serial.GetMessage<NDnnProto::TLayer>();
					if (GetRepeatedFieldSizeFromMessage(layerSpec, TConf::TSynapse::TConst::ProtoFieldNumber) > GetLocalId()) {
						PredefineSynapseConst.emplace(
							GetRepeatedFieldFromMessage<typename TConf::TSynapse::TConst::TProto>(
								layerSpec,
								TConf::TSynapse::TConst::ProtoFieldNumber,
								GetLocalId()
							)
						);
					}
				}
			}
			for (ui32 synId = 0; synId < Inner.SynapsesSize(); ++synId) {
				serial(Synapses[synId]);
			}
		}
		const TOptional<typename TConf::TSynapse::TConst::TProto>& GetPredefinedSynapseConst() const {
			return PredefineSynapseConst;
		}

		const TActVector<typename TConf::TSynapse>& GetSynapses() const {
		// const TVector<typename TConf::TSynapse>& GetSynapses() const {
			return Synapses;
		}

		const double& GetAxonDelay() const {
			return Inner.GetAxonDelay();
		}

	private:
		TPtr<TRandEngine> Rand;
		
		TAsyncSpikeQueue Queue;

		TNeuron Neuron;

		typename TConf::TActivationFunction Activation;
		TActVector<typename TConf::TSynapse> Synapses;
		// TVector<typename TConf::TSynapse> Synapses;

		TSpikeNeuronImplInner Inner;
		TNeuronSpaceInfo SpaceInfo;

		TOptional<typename TConf::TSynapse::TConst::TProto> PredefineSynapseConst;
	};

} // namespace NDnn