
#include <dnn/base/base.h>

#include <dnn/neuron/spike_neuron_impl.h>

#include <dnn/neuron/default_config.h>

namespace NDnn {

	template <typename N, ui32 size, typename TConf = TDefaultConfig>
	class TLayer: public IMetaProtoSerial {
	private:
		using TNeuronImplType = TSpikeNeuronImpl<N, TConf>;

	public:
		using TNeuronType = N;

		TLayer()
			: Id(0)
		{}

		ui32 Size() const {
			return size;
		}
		
		TNeuronImplType& operator[](ui32 id) {
			return Neurons[id];
		}

		void SetId(ui32 id) {
			Id = id;
		}

		const ui32& GetId() const {
			return Id;
		}
		auto begin() {
			return Neurons.begin();
		}
		auto end() {
			return Neurons.end();
		}

		void SerialProcess(TMetaProtoSerial& serial) override final {
			serial.DuplicateSingleRepeated(Size());
			for (auto& n: Neurons) {
				serial(n);
			}
		}

	private:
		ui32 Id;
		std::array<TNeuronImplType, size> Neurons;
	};



} // namespace NDnn