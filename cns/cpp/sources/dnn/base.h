#pragma once

#include "core.h"

namespace dnn {



template <typename Eval>
class EvalObject {
public:
	typedef Eval eval_type;
    virtual Eval eval() = 0;
};


template<typename State, typename Eval>
class DynamicObject : public EvalObject<Eval> {
public:
    virtual void step(State &dState_dt) = 0;
protected:
    State state;
};

template <typename Elem>
class System : public Object {
public:
	void add_elem(unique_ptr<Elem> e) {
		sys.push_back(std::move(e));
	}
protected:
    vector< unique_ptr<Elem> > sys;
};


template <typename Elem, typename Eval>
class EvalSystem : public EvalObject<Eval>, public System<Elem> {

};

template <int StateSize, typename StateType = double>
class BaseState : private vector<StateType> {
public:
    BaseState() : vector<StateType>(StateSize) {}
};


}
