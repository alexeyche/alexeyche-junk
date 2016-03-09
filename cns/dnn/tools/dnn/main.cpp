
#include <iostream>

#include <dnn/util/log/log.h>
#include <dnn/sim/sim.h>
#include <dnn/neuron/integrate_and_fire.h>

using namespace NDnn;

int main(int argc, const char** argv) {
	TLog::Instance().SetLogLevel(TLog::DEBUG_LEVEL);

	TSim<
		TLayer<TIntegrateAndFire, 100>,
		TLayer<TIntegrateAndFire, 100>
	>().Run();
    return 0;
}
