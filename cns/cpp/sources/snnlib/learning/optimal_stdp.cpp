
#include "optimal_stdp.h"


OptimalStdpStat::OptimalStdpStat(OptimalStdp *m) : Serializable<Protos::OptimalStdpStat>(EOptimalStdpStat) {
	for(size_t i=0; i < m->C.size(); i++) {
		C.push_back(vector<double>());
	}
}

void OptimalStdpStat::collect(OptimalStdp *m) {
	if(B.size() > STAT_COLLECT_LIMIT) return;
	for(size_t syn_i=0; syn_i < m->C.size(); syn_i++) {
    	C[syn_i].push_back(m->C[syn_i]);
    }
    B.push_back(m->B);
    p_acc.push_back(m->p_acc);
}
