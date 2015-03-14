#pragma once

#include <dnn/io/serialize.h>


namespace dnn {

struct InputInterface {
    retDoubleDelegate getValue;
};



class InputBase : public SerializableBase {
public:
    typedef InputInterface interface;

    virtual double getValue() = 0;
    virtual void provideInterface(InputInterface &i) = 0;

    static double getValueDefault() {
        return 0.0;
    }
    static void provideDefaultInterface(InputInterface &i) {
        i.getValue = &InputBase::getValueDefault;
    }
    //virtual void setExternalSource(Stream *s) {
//
 //   }
};



template <typename Constants, typename State>
class Input : public InputBase {
public:    
    void serial_process() {
        begin() << "Constants: " << c;

        if (messages->size() == 0) {
            (*this) << Self::end;
            return;
        }

        (*this) << "State: " << s << Self::end;;
    }
protected:
    Constants c;
    State s;
};


}
