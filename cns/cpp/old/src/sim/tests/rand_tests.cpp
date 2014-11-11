
#include <sim/util/rand/common_rand_gen.h>


void test_sample_rand_gen() {	
	SampleRandGen<vec> s("1 2 3 4 5", "0.1 0.2 0.3 0.4 0.2");
	vec out = s.gen(100);
	uvec counts;
	counts.zeros(5);
	for(size_t i=0; i<out.n_elem; i++) {		
		counts(int(out(i))-1)++;		
	}
	counts.print();
}
