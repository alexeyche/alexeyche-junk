
#include "test.h"
using namespace dnn;

void test() {
    Input i;
    LeakyIntegrateAndFire<Input> d(i);
    ActivFunc<LeakyIntegrateAndFire<Input>> f(d);
    cout << f.eval();

}
