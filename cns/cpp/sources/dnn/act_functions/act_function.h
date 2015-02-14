#pragma once

namespace dnn {

template <typename Constants, typename Eval>
class ActFunction : public DynamicObject<Eval> {
public:
	ActFunction(Constants _c) : c(_c) {}	
protected:
	Constants c;
};

}