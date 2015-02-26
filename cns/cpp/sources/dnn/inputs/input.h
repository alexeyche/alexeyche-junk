#pragma once



namespace dnn {

struct InputInterface {
    retDoubleDelegate getValue;
};


class InputBase {
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
};



template <typename Constants, typename State>
class Input : public InputBase {
protected:
    const Constants c;
    State s;
};


}
