
#include "common_rand_gen.h"


void test_sample_rand_gen() {	
	s = SampleRandGen({1,2,3,4,5}, {0.1,0.2,0.3,0.4,0.2});
	vec out = s.gen(100);
	out.print();
}
