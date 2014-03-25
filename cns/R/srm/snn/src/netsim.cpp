
#include "netsim.h"
#include "layer.h"

bool compare_syn_spikes (TSynSpike i, TSynSpike j) { return (i.t<j.t); }

NetSim::NetSim(List &net_, const vector<Layer*> &layers) : cons(net_.size()), queue_of_spikes(net_.size()), queue_of_input_spikes(net_.size()), spikes_iter(net_.size(), arma::fill::zeros), input_spikes_iter(net_.size(), arma::fill::zeros), net(net_) {
        for(size_t li=0; li<layers.size(); li++) {
            const Layer *l = layers[li];
            const arma::uvec ids = l->getIds();
            for(size_t ni=0; ni<l->N; ni++) {
                const arma::uvec id_conns = l->getConns(ni);
                for(size_t con_i=0; con_i < id_conns.n_elem; con_i++) {
                    cons[id_conns(con_i)-1].push_back(TConseq(ids(ni)-1, con_i) );
                }
            }
        }
        //for(size_t cons_i=0; cons_i<cons.size(); cons_i++) {
        //    cout << "cons_i(" << cons_i << ")";
        //    for(size_t syn_i=0; syn_i < cons[cons_i].size(); syn_i++) {
        //        cout << cons[cons_i][syn_i].nid << "|" << cons[cons_i][syn_i].syn_id << ",";
        //    }
        //    cout << "\n";
        //}
        for(size_t ss_i=0; ss_i < net.size(); ss_i++) {
            vector<double> sp = as<vector<double> >(net[ss_i]);
            for(size_t con_i=0; con_i<cons[ss_i].size(); con_i++) {
                for(size_t sp_i = 0; sp_i<sp.size(); sp_i++) {
                    queue_of_input_spikes[ cons[ss_i][con_i].nid ].push_back( TSynSpike( sp[sp_i], cons[ss_i][con_i].syn_id ) );
                }
            }
        }
        for(size_t sp_i=0; sp_i < queue_of_input_spikes.size(); sp_i++) {
            sort(queue_of_input_spikes[sp_i].begin(), queue_of_input_spikes[sp_i].end(), compare_syn_spikes);
            //cout << "sp_i: " << sp_i << "|";
            //for(size_t qi=0; qi<queue_of_input_spikes[sp_i].size(); qi++) {
            //    cout << "t:"  << queue_of_input_spikes[sp_i][qi].t << ",i:" << queue_of_input_spikes[sp_i][qi].syn_id << " ";
            //}
            //cout << "\n";
        }
}

TSynSpikes NetSim::getSpikes(size_t ni, const double &t, const double &dt) {
    TSynSpikes ssp;        
    size_t c_id = ni-1;
    if (queue_of_input_spikes[c_id].size() > 0) {
        //cout << "t  " << t << ": Found spikes at synapse " << queue_of_input_spikes[c_id][ input_spikes_iter(c_id) ].second << " of " << c_id << " at " << queue_of_input_spikes[c_id][ input_spikes_iter(c_id) ].first << "\n";
        while(    (queue_of_input_spikes[c_id].size()>input_spikes_iter(c_id) ) && 
                ( (queue_of_input_spikes[c_id][ input_spikes_iter(c_id) ].t >= t) && (queue_of_input_spikes[c_id][ input_spikes_iter(c_id) ].t < t+dt) )
             )
        {
            ssp.push_back(queue_of_input_spikes[c_id][ input_spikes_iter(c_id) ]);
            input_spikes_iter(c_id) += 1;
        }
    }
    if (queue_of_spikes[c_id].size() > 0) {
        //cout << "t  " << t << ": Found spikes at synapse " << queue_of_spikes[c_id][ spikes_iter(c_id) ].second << " of " << c_id << " at " << queue_of_spikes[c_id][ spikes_iter(c_id) ].first << "\n";
        while(    (queue_of_spikes[c_id].size()>spikes_iter(c_id) ) && 
                ( (queue_of_spikes[c_id][ spikes_iter(c_id) ].t >= t) && (queue_of_spikes[c_id][ spikes_iter(c_id) ].t < t+dt) )
             )
        {
            ssp.push_back(queue_of_spikes[c_id][ spikes_iter(c_id) ]);
            spikes_iter(c_id) += 1;
        }
    }    
    return ssp;
}


void NetSim::prop_spike(size_t id, double t) {
    size_t c_id = id-1;
    NumericVector sp = net[c_id];
    sp.push_back(t);
    net[c_id] = sp;

    for(size_t con_i=0; con_i<cons[c_id].size(); con_i++) {
//        cout << "id: " << id <<  " c_id: " << c_id << " con_i: " << con_i << " i: " << cons[c_id][con_i].nid << "\n";
        queue_of_spikes[ cons[c_id][con_i].nid ].push_back(TSynSpike(t, cons[c_id][con_i].syn_id));
    }
}

