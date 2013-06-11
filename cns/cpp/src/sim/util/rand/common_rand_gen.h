#ifndef COMMON_RAND_GEN_H
#define COMMON_RAND_GEN_H


#include <sim/core.h>


template <typename TSample>
class RandGen {
public:
	virtual TSample gen(int n) = 0;
};

template <typename TSample>
class UnifRandGen : public RandGen<TSample> {
public:	
	UnifRandGen(double min, double max) : min(min), max(max) {}
	TSample gen(int n) {
		std::srand(time(NULL));
		TSample out;
		out.randu(n);
		out = min + max * out;
		return out;
	}
	double min;
	double max;
};

template <typename TSample>
class SampleRandGen : public RandGen<TSample> {
public:
	SampleRandGen(TSample samples_c, vec probs_c) : samples(samples_c), probs(probs_c) { 
		if(samples.n_elem != probs.n_elem) {
			throw std::logic_error("Length of probs and samples must be equal"); 
		}
		probs=probs/sum(probs);
		uvec ind = sort_index(probs, 1);
		probs = probs(ind);
		samples = samples(ind);
	}
	SampleRandGen(TSample samples_c) : samples(samples_c), probs(samples_c.n_elem) { 		
		probs.fill(1.0/samples_c.n_elem);		
	}
	SampleRandGen(int min, int max) : samples(max-min+1), probs(max-min+1) {
		int ind = 0;
		for(int i=min; i<=max; i++) {
			probs(ind) = 1.0/(max-min);
			samples(ind) = i;
			ind++;
		}		
	}

	TSample gen(int n) {
		TSample out;
		std::srand(time(NULL));
		vec r, cum_p;
		
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
	TSample samples;
	vec probs;
};

#endif