#pragma once



namespace dnn {

template <typename Constants, typename State>
class Input : public DynamicObject<double>, public StateObject<Constants, State> {
public:
	Input() {}
};


}
