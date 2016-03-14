#pragma once

#include <dnn/base/base.h>
#include <dnn/neuron/spike_neuron_impl.h>
#include <dnn/neuron/default_config.h>
#include <dnn/connection/builder.h>

#include <array>

namespace NDnn {

	template <typename N, ui32 size, typename TConf = TDefaultConfig>
	class TLayer: public IMetaProtoSerial {
	private:
		using TNeuronImplType = TSpikeNeuronImpl<N, TConf>;

	public:
		using TNeuronType = N;
		using TNeuronArr = std::array<TNeuronImplType, size>;

		TLayer()
			: Id(0)
		{}

		ui32 Size() const {
			return size;
		}

		TNeuronImplType& operator[](ui32 id) {
			return Neurons[id];
		}

		void SetupSpaceInfo(ui32 id, ui32 prevLayerNeuronsSize) {
			Id = id;
	        ui32 rowId = 0;
        	ui32 colId = 0;
        	ui32 colSize = ceil(sqrt(Size()));
			for (ui32 nId=0; nId < Neurons.size(); ++nId) {
				TNeuronSpaceInfo info;
				info.LayerId = Id;
				info.LocalId = nId;
				info.GlobalId = prevLayerNeuronsSize + nId;
				info.ColumnSize = colSize;
				info.RowId = rowId;
				info.ColId = colId;

				Neurons[nId].SetSpaceInfo(info);

				rowId++;
                if(rowId % colSize == 0) {
                    colId++;
                    rowId = 0;
                }
			}
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
		template <typename TDstLayer>
		void Connect(TDstLayer& dstLayer, const NDnnProto::TConnection& conn, TRandEngine& rand) {
			auto connectionPtr = BuildConnection(conn, rand);

			for (auto& npre : GetNeurons()) {
				for (auto& npost : dstLayer.GetMutNeurons()) {
					if (npre == npost) {
						continue;
					}
					auto connRecipe = connectionPtr->GetConnectionRecipe(npre.GetSpaceInfo(), npost.GetSpaceInfo());
					if (!connRecipe.Exists) {
						continue;
					}

					typename TConf::TSynapse syn;
					auto synConst = npost.GetPredefinedSynapseConst();
					if (synConst) {
						TProtoSerial serial(*synConst, TSerialBase::ESerialMode::IN);
						syn.MutConstants().SerialProcess(serial);
					}

					syn.MutWeight() = connRecipe.Amplitude * conn.weight();
					syn.MutIdPre() = npre.GetGlobalId();
					syn.MutDendriteDelay() = conn.dendritedelay();

					npost.AddSynapse(std::move(syn));
				}
			}

		}
		const TNeuronArr& GetNeurons() const {
			return Neurons;
		}

		TNeuronArr& GetMutNeurons() {
			return Neurons;
		}

	private:
		ui32 Id;
		TNeuronArr Neurons;
	};



} // namespace NDnn