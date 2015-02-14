#pragma once


namespace dnn {

template <typename Eval>
class DynamicObject {
public:
	virtual Eval evaluate() = 0;
};

}