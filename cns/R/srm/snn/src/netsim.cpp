
#include "netsim.h"
#include "layer.h"

bool compare_syn_spikes (TSynSpike i, TSynSpike j) { return (i.first<j.first); }

NetSim::NetSim(List &net_, const vector<Layer*> &layers) : cons(net_.size()), queue_of_spikes(net_.size()), spikes_iter(net_.size(), arma::fill::zeros), net(net_) {
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
        for(size_t ss_i=0; ss_i < net.size(); ss_i++) {
            vector<double> sp = as<vector<double> >(net[ss_i]);
            for(size_t con_i=0; con_i<cons[ss_i].size(); con_i++) {
                for(size_t sp_i = 0; sp_i<sp.size(); sp_i++) {
                    queue_of_spikes[ cons[ss_i][con_i].first ].push_back( TSynSpike( sp[sp_i], cons[ss_i][con_i].second) );

                }
            }
        }
        for(size_t sp_i=0; sp_i < queue_of_spikes.size(); sp_i++) {
            sort(queue_of_spikes[sp_i].begin(), queue_of_spikes[sp_i].end(), compare_syn_spikes);
            cout << "sp_i: " << sp_i << "|";
            for(size_t qi=0; qi<queue_of_spikes[sp_i].size(); qi++) {
                cout << queue_of_spikes[sp_i][qi].first << "," << queue_of_spikes[sp_i][qi].second << " ";
            }
            cout << "\n";
        }
}
TSynSpikes NetSim::getSpikes(size_t ni, const double &t, const double &dt) {
//    cout << "Looking for spikes for ni = " << ni << " at " << t << "| queue_of_spikes[ni-1].size() = " << queue_of_spikes[ni-1].size() << " spikes_iter[ni-1] = " << spikes_iter[ni-1] << "\n"; 
    TSynSpikes ssp;        
    if((queue_of_spikes[ni-1].size() > 0)&&(queue_of_spikes[ni-1].size()>spikes_iter(ni-1))) {
        cout << "t  " << t << ": Found spikes at synapse " << queue_of_spikes[ni-1][ spikes_iter(ni-1) ].second << " of " << ni-1 << " at " << queue_of_spikes[ni-1][ spikes_iter(ni-1) ].first << "\n";
        while(    (queue_of_spikes[ni-1].size()>spikes_iter(ni-1) ) && 
                ( (queue_of_spikes[ni-1][ spikes_iter(ni-1) ].first > t) && (queue_of_spikes[ni-1][ spikes_iter(ni-1) ].first <= t+dt) )
             )
        {
            ssp.push_back(queue_of_spikes[ni-1][ spikes_iter(ni-1) ]);
            spikes_iter(ni-1) += 1;
        }
    }
    return ssp;
}


void NetSim::prop_spike(const size_t &id, const double &t) {
    size_t c_id = id-1;
    NumericVector sp = net[c_id];
    sp.push_back(t);
    net[c_id] = sp;
    
    for(size_t con_i=0; cons[id-1].size(); con_i++) {
        queue_of_spikes[ cons[id-1][con_i].first ].push_back(TSynSpike(t, cons[id-1][con_i].second));
    }
}

