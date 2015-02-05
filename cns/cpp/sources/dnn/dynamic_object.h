#pragma once

#include "core.h"
#include "active_container.h"

namespace dnn {

//DynamicObject returns State
//System returns States

// Input ->|
//         |DynamicObject-> FunEventGenerator
// System->|
//
// or
//
// Fun( DynamicObject(Fun(Input), System) )

// Fun1< DynamicObject0<Input, Fun0<System0>> > sys = funEval(Do2InputEval(inputEval(), funEval(System)));
// sys.eval();
//




// template <typename Arg>
// class LeakyIntegrateAndFire : public DynamicObjectDep<Arg> {
// public:
//     LeakyIntegrateAndFire(Arg &o) : DynamicObjectDep<Arg>(o) {}

//     double eval() {
//         double o_ev = DynamicObjectDep<Arg>::o.eval();
//         return o_ev*2;
//     }

//     void step(double &dState_dt) {
//         dState_dt += DynamicObject::state/100.0;
//     }


// };

// template <typename Arg>
// class ActivFunc : public ObjectDep<Arg> {
// public:
//     ActivFunc(Arg &_o) : ObjectDep<Arg>(_o) {}
//     double eval() {
//         double u = ObjectDep<Arg>::o.eval();
//         if(u>5.0) {
//             return 1.0;
//         }
//         return 0.0;
//     }
// };



}
