
#include "test.h"
using namespace dnn;

void test() {
	StaticSynapseSystem s;
	s.add_elem(unique_ptr<StaticSynapse>(new StaticSynapse));
	InputTimeSeries ts;
	LeakyIntegrateAndFire lif(ts, s);
    Determ act_f(lif);
    cout << act_f.eval();

}
