#ifndef SYNAPSE_H
#define SYNAPSE_H

#include <sim/main/connection.h>
#include <sim/util/rand/rand_funcs.h>

#include <sim/util/debug_funcs.h>
#include <sim/socket/sim_socket_core.h>

#define START_SYN_STRENGTH 0.1

enum SynType { NMDA = 1, AMPA = 2, GABA_A = 3, GABA_B = 4};

class SynapticOptions {
public:	
	SynapticOptions(uvec indices_in, uvec indices_out, double syn_strength, double syn_num, SynType st, double syn_tau) : 
					indices_in(indices_in),
					indices_out(indices_out),
					syn_strength(syn_strength),
					syn_num(syn_num),
					st(st),
					syn_tau(syn_tau)

	{

	}
	uvec indices_in;
	uvec indices_out;
	double syn_strength;
	double syn_num;
	SynType st;
	double syn_tau;
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
													    syn_potential(pre->size.n_out, post->size.n_in),
													    syn_conn(pre->size.n_out, post->size.n_in),
													    out_to_r(post->size.n_in),
													    nmda_con(pre->size.n_out, post->size.n_in),
													    gaba_a_con(pre->size.n_out, post->size.n_in)

	{
		ltp.fill(0);
		ltd.fill(0);
		sd.fill(0);		
		s.fill(0);
		g.fill(0);
		g_tau.fill(1);
		syn_types.fill(0);
		syn_potential.fill(0);
		out_to_r.fill(0);
		syn_conn.fill(0);
		nmda_con.fill(0);
		gaba_a_con.fill(0);
		for(size_t i=0; i<sgo.group_size(); i++) {			
			for(size_t j=0; j < sgo[i]->indices_in.n_elem; j++) {
				uvec ind_row(1);
				ind_row.fill(sgo[i]->indices_in(j));			

				uvec null_conn = find(s(ind_row,sgo[i]->indices_out) == 0);
				
				uvec ind = get_shuffled_indices(null_conn.n_elem);			
				ind = ind(span(0, sgo[i]->syn_num));	
				uvec ind_out = null_conn(ind);
				
				s(ind_row,ind_out).fill(sgo[i]->syn_strength);
				g_tau(ind_row, ind_out).fill(sgo[i]->syn_tau);				
				syn_types(ind_row, ind_out).fill(sgo[i]->st);
				syn_conn(ind_row, ind_out).fill(1);
				if(sgo[i]->st == NMDA) { 
					nmda_con(ind_row, ind_out).fill(1);
				}			
				if(sgo[i]->st == GABA_A) { 
					gaba_a_con(ind_row, ind_out).fill(1);
				}			
			}

		}
		// system stuff
		ampa_col = find(syn_types == AMPA);
		nmda_col = find(syn_types == NMDA);		
		gaba_a_col = find(syn_types == GABA_A);
		gaba_b_col = find(syn_types == GABA_B);
		
		
		tau_ltp = sgo.tau_ltp;
		tau_ltd = sgo.tau_ltd;
		syn_max = sgo.syn_max;
		send_i=0;
	}

	void computeMe(double dt) {

		uvec fired = find(in);
		if(fired.n_elem>0) {
			Timer::Start("cycle1");
			// syn_cons_id.cols(fired)
			// 
			
			// 5 6 1 2 7
			// 1 2 5 3 8
			mat in_rep = repmat(in,1,in.n_rows);
		    mat nmda_pot = in_rep % nmda_con;
		    mat gaba_a_pot = in_rep % gaba_a_con;
		    
		    // various synapses
		    //pot(ampa_col) = pot(ampa_col);
		    
		    mat nmda_inter = pow((nmda_pot+80)/60, 2);
		    nmda_pot = nmda_pot % nmda_inter/(1+nmda_inter);
		    gaba_a_pot = gaba_a_pot - 70;
		    //pot(gaba_a_col) = pot(gaba_a_col) - 70;			
			//pot(gaba_b_col) = pot(gaba_b_col) - 90;
		    mat full_syn = gaba_a_pot + nmda_pot;
		    uvec post_ind_cur = find(full_syn);
		    g(post_ind_cur) = s(post_ind_cur);		    						
			
			syn_potential +=  full_syn;
		//	print_m(syn_potential);
			in(fired).fill(0);
			Timer::Stop("cycle1");
		} 
		uvec post_ind = find(syn_potential);
		if (post_ind.n_elem > 0) {    
			Timer::Start("cycle2");
			syn_potential = syn_potential % g;
			syn_potential(find(syn_potential == 0)).fill(0);
			// out_to_r = syn_potential;
			// send_arma_mat(out_to_r,"syn",&send_i,true);
			// std::cin.ignore();
			// send_i++;
			
			//print_m(g);
			out = sum(syn_potential, 0);			
			Timer::Stop("cycle2");
		} else {
			g.fill(0);
		}
		Timer::Start("cycle3");
		g = g - dt * g/g_tau;   // element-wise division
		Timer::Stop("cycle3");
	}

	vec out_to_r; long unsigned int send_i;
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
	
	mat syn_conn; // system stuff for vector calc
	uvec ampa_col;
	uvec gaba_a_col;
	uvec nmda_col;
	uvec gaba_b_col;	
	mat nmda_con;		
	mat gaba_a_con;		
};


#endif