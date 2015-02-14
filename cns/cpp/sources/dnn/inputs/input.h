#pragma once



namespace dnn {

template <typename Constants, typename State>
class Input : public DynamicObject<double> {
public:
	Input(Constants _c) : c(_c) {}
protected:
	Constants c;
	State s;
};


}