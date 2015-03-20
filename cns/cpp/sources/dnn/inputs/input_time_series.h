#pragma once

#include "input.h"

#include <dnn/protos/generated.pb.h>
#include <dnn/io/stream.h>

namespace dnn {

/*@GENERATE_PROTO@*/
struct InputTimeSeriesC : public Serializable<Protos::InputTimeSeriesC> {
    InputTimeSeriesC() : dt(1.0) {}
    
    double dt;
    
    void serial_process() {
        begin() << "dt: " << dt << Self::end;
    }
};

/*@GENERATE_PROTO@*/
struct InputTimeSeriesState : public Serializable<Protos::InputTimeSeriesState> {
    InputTimeSeriesState() : index(0), _t(0) {}
    size_t index;
    double _t;
    void serial_process() {
        begin() << "index: " << index << Self::end;
    }
};


class InputTimeSeries : public Input<InputTimeSeriesC, InputTimeSeriesState> {
public:
    typedef Input<InputTimeSeriesC, InputTimeSeriesState> Parent;

    const string name() const {
        return "InputTimeSeries";
    }

	const double& getValue(const Time &t) {
        s._t += t.dt;
        if(fmod(s._t, c.dt) > 0.0001) return InputBase::def_value;        
        return ts.ref().data.values[s.index++];
	}
    void provideInterface(InputInterface &i) {
        i.getValue = MakeDelegate(this, &InputTimeSeries::getValue);
    }

    void setTimeSeries(TimeSeries *_ts) {
        s.index = 0;
        ts.set(_ts);
    }
    double getDuration() {
        if(ts.isSet()) {
            return ts.ref().length() * c.dt;
        }
        return 0.0;
    }
private:
    InterfacedPtr<TimeSeries> ts;
};



}
