
#include "test.h"
using namespace dnn;

void test() {
	InputTimeSeries ts;
	LeakyIntegrateAndFire lif(ts);
    cout << lif.eval();

}
