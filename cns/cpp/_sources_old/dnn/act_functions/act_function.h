#pragma once

namespace dnn {

template <typename Constants, typename Eval>
class ActFunction : public StatelessObject<Constants>, public DynamicObject<Eval> {
public:
	ActFunction() {}
};

}
