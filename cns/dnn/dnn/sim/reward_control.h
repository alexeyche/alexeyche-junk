#pragma once

#include <atomic>

#include <dnn/protos/reward_control.pb.h>
#include <dnn/util/serial/proto_serial.h>

#include <dnn/protos/config.pb.h>

namespace NDnn {

    namespace {

        double AtomicDoubleAdd(std::atomic<double> &f, double d) {
            double old = f.load(std::memory_order_consume);
            double desired =  old+d;
            while (!f.compare_exchange_weak(
                    old
                  , desired
                  , std::memory_order_release
                  , std::memory_order_consume
            )) {
                desired = old + d;
            }
            return desired;
        }

    } // namespace

    struct TRewardControlState : public IProtoSerial<NDnnProto::TRewardControlState> {
        void SerialProcess(TProtoSerial& serial) override {
            serial(R);
            serial(MeanR);
        }

        double R = 0.0;
        double MeanR = 0.0;
    };

    struct TRewardControlConst : public IProtoSerial<NDnnProto::TRewardControlConst> {
        void SerialProcess(TProtoSerial& serial) override {
            serial(TauTrace);
            serial(TauMeanTrace);
        }


        double TauTrace = 100.0;
        double TauMeanTrace = 10000.0;
    };



    class TRewardControl : public IProtoSerial<NDnnProto::TRewardControl> {
    public:
        TRewardControl()
            : GatheredReward(0.0)
        {}

        void CalculateDynamics(const TTime& t) {
            s.MeanR += t.Dt * ( - (s.MeanR - s.R)/c.TauMeanTrace );
            s.R += t.Dt * ( - (s.R - GatheredReward.load())/c.TauTrace );

            GatheredReward.store(0.0);
        }

        void SerialProcess(TProtoSerial& serial) override {
            serial(c);
            serial(s);
        }

        void GatherReward(double r) {
            AtomicDoubleAdd(GatheredReward, r);
        }

        TRewardControl& operator = (const TRewardControl &other) {
            if (this != &other) {
                GatheredReward.store(other.GatheredReward.load());
                s = other.s;
                c = other.c;
            }
            return *this;
        }

        TRewardControl(const TRewardControl &other) {
            (*this) = other;
        }
        
        const double& GetReward() const {
            return s.R;
        }
        
        double GetRewardDelta() const {
            return s.R - s.MeanR;
        }
    private:
        std::atomic<double> GatheredReward;

        TRewardControlState s;
        TRewardControlConst c;
    };




}