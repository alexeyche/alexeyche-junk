

#include <sim/main/axon.h>

void axon_test() {
	
	vec V_in_vec(1000);
	V_in_vec.fill(0);
	
	AxonDelay ax(1000, V_in_vec);
	
	AxonDelayOptions ado(1000);
	ado.setGenerator(new UnifRandGen<vec>(1,20));
	ax.prepareMe(&ado);
	
	SampleRandGen<uvec> sr(1,400);
	uvec ind = sr.gen(400);
	V_in_vec(ind).fill(30);

	Timer::Start("Axon compute");
	for(double t=0; t<20; t++) {		
		Timer::Start("Current timestep axon compute");
		ax.computeMe(1);
		Timer::Stop("Current timestep axon compute");
	}
	Timer::Stop("Axon compute");
	
	vec out = ax.getOutput();		
	for(size_t i=0; i<out.n_elem; i++) {
		if( out(i) != 0) {
			std::cout << i << ":" << out(i) << " ";
		}
	}
	std::cout << std::endl;
}