#pragma once


#include "activation.h"

#include <dnn/util/serial.h>
#include <dnn/protos/determ.pb.h>

namespace NDnn {

    struct TDetermConst: public IProtoSerial<NDnnProtos::TDetermConst> {
        TDetermConst() : Threshold(1.0) {}

        void SerialProcess(TProtoSerial& serial) override final {
            serial(Threshold);
        }

        double Threshold;
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
