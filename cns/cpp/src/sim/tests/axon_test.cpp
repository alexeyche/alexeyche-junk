

#include <sim/main/axon.h>

void axon_test() {
	AxonDelay ax(100);
	AxonDelayOptions ado(100);
	ado.setGenerator(new UnifRandGen(1,20));
	ax.prepareMe(&ado);
	
	double in = 10;
	SampleRandGen sr(1,20);
	vec v = sr.gen(20);
	v.print();
	uvec ind = conv_to<uvec>::from(v);
	ax.I_in(ind).fill(in);	
	for(double t=0; t<10; t++) {		
		ax.computeMe(t);
	}
	ax.I_out.print();
}