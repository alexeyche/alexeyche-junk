#pragma once


#include "activation.h"

#include <dnn/util/serial/proto_serial.h>
#include <dnn/protos/determ.pb.h>

namespace NDnn {

    struct TDetermConst: public IProtoSerial<NDnnProto::TDetermConst> {
        static const auto ProtoFieldNumber = NDnnProto::TLayer::kDetermConstFieldNumber;

        void SerialProcess(TProtoSerial& serial) override final {
            serial(Threshold);
        }

        double Threshold = 1.0;
    };


    class TDeterm : public TActivation<TDetermConst> {
    public:
        double SpikeProbability(const double &u) {
            if(u >= c.Threshold) {
                return 1.0;
            }
            return 0.0;
        }

        double SpikeProbabilityDerivative(const double &u) {
            return 0.0;
        }
    };



}
