#pragma once


template <typename Eval>
class Object {
public:
    virtual Eval eval() = 0;
};

template <typename Arg, typename Eval>
class ObjectDep : public Object<Eval> {
public:
    ObjectDep(Arg _o)  : o(_o) {}
protected:
    Arg o;
};
template<typename State, typename Eval>
class DynamicObject : public Object<Eval> {
public:
    virtual void step(State &dState_dt) = 0;
protected:
    State state;
};

template <typename Arg, typename State, typename Eval>
class DynamicObjectDep : public DynamicObject<State, Eval>, public ObjectDep<Arg, Eval> {
public:
    DynamicObjectDep(Arg _o) : ObjectDep<Arg, Eval>(_o) {}
};


template <typename State, typename Eval>
class System : public Object<Eval> {
public:
    vector< unique_ptr<DynamicObject<State,Eval>> > sys;
};

template <int StateSize, typename StateType = double>
struct State : public vector<StateType> {
    State() : vector<StateType>(StateSize) {}
};
