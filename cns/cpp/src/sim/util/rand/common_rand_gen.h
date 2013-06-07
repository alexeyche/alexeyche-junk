#ifndef COMMON_RAND_GEN_H
#define COMMON_RAND_GEN_H


#include <sim/core.h>


class RandGen {
public:
	virtual vec gen(int n) = 0;
};

class UnifRandGen : public RandGen {
public:	
	UnifRandGen(double min, double max) : min(min), max(max) {}
	vec gen(int n) {
		std::srand(time(NULL));
		vec out;
		out.randu(n);
		out = min + max * out;
		return out;
	}
	double min;
	double max;
};

class SampleRandGen : public RandGen {
public:
	SampleRandGen(vec samples_c, vec probs_c) : samples(samples_c), probs(probs_c) { 
		if(samples.n_elem != probs.n_elem) {
			throw std::logic_error("Length of probs and samples must be equal"); 
		}
		probs=probs/sum(probs);
		uvec ind = sort_index(probs, 1);
		probs = probs(ind);
		samples = samples(ind);
	}
	SampleRandGen(int min, int max) : probs(max-min+1), samples(max-min+1) {
		int ind = 0;
		for(int i=min; i<max; i++) {
			probs(i) = 1.0/(max-min);
			samples(ind++) = i; 
		}
		probs.print();
		samples.print();
	}
	vec gen(int n) {
		std::srand(time(NULL));
		vec r, cum_p, out;
		r.randu(n);		
		cum_p.zeros(n);
		out.zeros(n);
		for(size_t i=0; i<samples.n_elem; i++) {			
			cum_p += probs(i);			
			uvec fired_ind = find( (cum_p-r)>0 );			
			out(fired_ind).fill(samples(i));
			cum_p(fired_ind).fill(-1);
		}		
		return out;
	}
	vec samples;
	vec probs;
};

#endif