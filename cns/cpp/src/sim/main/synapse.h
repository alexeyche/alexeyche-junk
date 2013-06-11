#ifndef SYNAPSE_H
#define SYNAPSE_H

#include <sim/main/connection.h>
#include <sim/util/rand/rand_funcs.h>

class SynapticOptions {
public:	
	SynapticOptions(uvec indices_in, uvec indices_out, double syn_strength = 6, double syn_num = 300) : 
					indices_in(indices_in),
					indices_out(indices_out),
					syn_strength(syn_strength),
					syn_num(syn_num)																					 
	{

	}
	uvec indices_in;
	uvec indices_out;
	double syn_strength;
	double syn_num;
};
class SynapticGroupOptions {
public:	
	SynapticGroupOptions(double tau_ltp = 20, double tau_ltd = 20, double syn_max = 10) :
													tau_ltp(tau_ltp),
													tau_ltd(tau_ltd),
													syn_max(syn_max) {}
	void add(SynapticOptions *so) {
		syn_opts.push_back(so);
	}
	int group_size() { return syn_opts.size(); }   
	SynapticOptions* operator [] (int i) { return syn_opts[i]; }
	double tau_ltp;
	double tau_ltd;
	double syn_max;
private:
	std::vector<SynapticOptions*> syn_opts;	
};

class Synapse : public Connection {
public:
	Synapse(SimElemCommon* pre, SimElemCommon* post,  SynapticGroupOptions &sgo) :  Connection(pre,post), 
													    ltp(post->size.n_in), 
													    ltd(post->size.n_in),
													    s(pre->size.n_out, post->size.n_in),
													    sd(pre->size.n_out, post->size.n_in)
	{
		ltp.fill(0);
		ltd.fill(0);
		sd.fill(0);
		for(size_t i=0; i<sgo.group_size(); i++) {			
			for(size_t j=0; j< sgo[i]->indices_in.n_elem; j++) {
				uvec ind = get_shuffled_indices(sgo[i]->indices_out.n_elem);			
				ind = ind(span(0, sgo[i]->syn_num));				
				uvec ind_row(1);
				ind_row.fill(sgo[i]->indices_in(j));			
				s(ind_row,ind).fill(sgo[i]->syn_strength);
			}			
		}
		tau_ltp = sgo.tau_ltp;
		tau_ltd = sgo.tau_ltd;
		syn_max = sgo.syn_max;
	}

	void computeMe(double dt) {		
		uvec fired = find(in);
		if(fired.n_elem>0) {
			vec out_m = in.t() * s;
			out_m.print();
			std::cin.ignore();
		}
		g_ampa -= g_ampa/tau_ampa;
		ltp *= (1-1/tau_ltp);
		ltd *= (1-1/tau_ltd);
	}

private:
	vec ltp; // stdp
	vec ltd; 	
	mat s;	
	mat sd;
	double tau_ltp;
	double tau_ltd;
	double syn_max;
	mat g_ampa;
	mat g_nmda;
};


#endif