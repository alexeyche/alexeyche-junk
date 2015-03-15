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
    InputTimeSeriesState() : index(0) {}
    size_t index;

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

	double getValue() {
        return ts.ref().data.vals[s.index];
	}
    void provideInterface(InputInterface &i) {
        i.getValue = MakeDelegate(this, &InputTimeSeries::getValue);
    }

    void setTimeSeries(const string& filename, const string& format) {
        ts.set(Factory::inst().getCachedTimeSeries(name(), filename, format));
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
