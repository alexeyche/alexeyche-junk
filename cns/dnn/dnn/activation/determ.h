#pragma once


#include "activation.h"

namespace NDnn {

    struct TDetermC {
        TDetermC() : Threshold(1.0) {}

        double Threshold;
    };


    class TDeterm : public TActivation<TDetermC> {
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
