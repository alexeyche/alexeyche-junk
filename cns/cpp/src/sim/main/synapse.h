#ifndef SYNAPSE_H
#define SYNAPSE_H

#include <sim/main/connection.h>
#include <sim/util/rand/rand_funcs.h>

#define START_SYN_STRENGTH 0.1

enum SynType { NMDA = 1, AMPA = 2, NMDA_A = 3, AMPA_A = 4};

class SynapticOptions {
public:	
	SynapticOptions(uvec indices_in, uvec indices_out, double syn_strength = 6, double syn_num = 300, SynType st = NMDA ) : 
					indices_in(indices_in),
					indices_out(indices_out),
					syn_strength(syn_strength),
					syn_num(syn_num),
					st(st)																					 
	{

	}
	uvec indices_in;
	uvec indices_out;
	double syn_strength;
	double syn_num;
	SynType st;
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
													    g(pre->size.n_out, post->size.n_in),
													    g_tau(pre->size.n_out, post->size.n_in),
													    sd(pre->size.n_out, post->size.n_in),
													    syn_types(pre->size.n_out, post->size.n_in),
													    syn_potential(pre->size.n_out, post->size.n_in)
	{
		ltp.fill(0);
		ltd.fill(0);
		sd.fill(0);		
		s.fill(0);
		g.fill(0);
		g_tau.fill(0);
		syn_types.fill(0);
		syn_potential.fill(0);
		for(size_t i=0; i<sgo.group_size(); i++) {			
			for(size_t j=0; j < sgo[i]->indices_in.n_elem; j++) {
				uvec ind = get_shuffled_indices(sgo[i]->indices_out.n_elem);			
				ind = ind(span(0, sgo[i]->syn_num));	
				uvec ind_out = sgo[i]->indices_out(ind);
				uvec ind_row(1);
				ind_row.fill(sgo[i]->indices_in(j));			
				s(ind_row,ind_out).fill(sgo[i]->syn_strength);
				if(sgo[i]->st == AMPA) {
					g_tau(ind_row, ind_out).fill(5);					
				}					
				if(sgo[i]->st == AMPA_A) {
					g_tau(ind_row, ind_out).fill(6);
					
				}
				syn_types(ind_row, ind_out).fill(sgo[i]->st);
			}

		}
		// for(size_t i=0; i<s.n_cols;i++) {
		//  	for(size_t j=0; j<s.n_rows;j++) {
		// 		std::cout << syn_types(i,j) << "|";
		// 	}
		// 	std::cout<< std::endl;
		// }
		// std::cin.ignore();
		tau_ltp = sgo.tau_ltp;
		tau_ltd = sgo.tau_ltd;
		syn_max = sgo.syn_max;
	}

	void computeMe(double dt) {
		uvec fired = find(in);
		if(fired.n_elem>0) {
			//(in.t() - 0) * g_ampa
			//mat syn_fired = syn_types.rows(fired);
			//mat syn_p_fired = syn_potential.rows(fired);
			
			syn_potential(fired) += in;
			//uvec ampa_col = find(syn_fired == AMPA);
			//uvec ampa_a_col = find(syn_fired == AMPA_A);
			
			syn_potential.print();
			std::cin.ignore();
		}
		g = -g/g_tau;   // element-wise division
	}

private:
	vec ltp; // stdp
	vec ltd; 	
	mat s;	
	mat sd;
	double tau_ltp;
	double tau_ltd;
	double syn_max;
	mat g;	
	mat g_tau;
	mat syn_types;

	mat syn_potential;

};


#endif