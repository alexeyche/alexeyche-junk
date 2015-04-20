#pragma once

#include <dnn/io/serialize.h>
#include <dnn/util/time_series.h>

namespace dnn {


struct InputInterface {
    retRefDoubleAtTimeDelegate getValue;
};

class InputBase : public SerializableBase {
public:
    typedef InputInterface interface;
    

    virtual const double& getValue(const Time &t) = 0;
    
    template <typename T>
    void provideInterface(InputInterface &i) {
        i.getValue = MakeDelegate(static_cast<T*>(this), &T::getValue);
    }
    static const double def_value;
    static const double& getValueDefault(const Time &t) {
        return def_value;
    }

    static void provideDefaultInterface(InputInterface &i) {
        i.getValue = &InputBase::getValueDefault;
    }
};



// /*@GENERATE_PROTO@*/
// struct InputInfo : public Serializable<Protos::InputInfo> {
//     InputInfo() : layer_id(0) {}

//     void serial_process() {
//         begin() << "layer_id: " << layer_id << Self::end;
//     }

//     size_t layer_id;
// };

template <typename Constants, typename State>
class Input : public InputBase {
public:    
    void serial_process() {
        begin() << "Constants: " << c << ", ";

        if (messages->size() == 0) {
            (*this) << Self::end;
            return;
        }

        (*this) << "State: " << s << Self::end;
    }
    private:    
    
protected:
    Constants c;
    State s;
};


}
