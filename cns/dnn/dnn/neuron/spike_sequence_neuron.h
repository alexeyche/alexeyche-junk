#pragma once

#include "spike_neuron.h"

#include <dnn/util/serial/proto_serial.h>
#include <dnn/protos/spike_sequence_neuron.pb.h>
#include <dnn/protos/config.pb.h>

namespace NDnn {

    struct TSpikeSequenceNeuronConst: public IProtoSerial<NDnnProto::TSpikeSequenceNeuronConst> {
        static const auto ProtoFieldNumber = NDnnProto::TLayer::kSpikeSequenceNeuronConstFieldNumber;

        void SerialProcess(TProtoSerial& serial) override final {
            serial(Dt);
        }


        double Dt = 1.0;
    };


    struct TSpikeSequenceNeuronState: public IProtoSerial<NDnnProto::TSpikeSequenceNeuronState> {
        static const auto ProtoFieldNumber = NDnnProto::TLayer::kSpikeSequenceNeuronStateFieldNumber;

        void SerialProcess(TProtoSerial& serial) override final {
            serial(Index);
        }

        ui32 Index = 0;
    };

    class TSpikeSequenceNeuron : public TSpikeNeuron<TSpikeSequenceNeuronConst, TSpikeSequenceNeuronState> {
    public:
        void Reset() {
            s.Index = 0;
        }

        void PostSpikeDynamics(const TTime&) {}

        void CalculateDynamics(const TTime& t, double Iinput, double Isyn) {
            assert(Seq.IsSet());

            if (s.Index >= Seq->size()) {
                return;
            }

            const double& spike_time = Seq->at(s.Index);
            if ((spike_time>=t.T) && (spike_time<(t.T+t.Dt))) {
                s.Index++;
                MutFired() = true;
            }
        }

        void SetAsInput(const TVector<double>& seq) {
            Seq.Set(&seq);
        }

    private:
        TPtr<const TVector<double>> Seq;
    };

} // namespace NDnn
