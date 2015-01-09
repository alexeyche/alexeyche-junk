
#include "triple_stdp.h"


TripleStdpStat::TripleStdpStat(TripleStdp *m) : Serializable<Protos::TripleStdpStat>(ETripleStdpStat) {
	for(size_t i=0; i < m->r1.size(); i++) {
		r1.push_back(vector<double>());
		if(i<m->r2.size()) r2.push_back(vector<double>());
	}
}

void TripleStdpStat::collect(TripleStdp *m) {
	if(o1.size() > STAT_COLLECT_LIMIT) return;
	for(size_t syn_i=0; syn_i < m->r1.size(); syn_i++) {
    	r1[syn_i].push_back(m->r1[syn_i]);
    	if(syn_i<m->r2.size()) {
    		r2[syn_i].push_back(m->r2[syn_i]);
    	}
    }
    o1.push_back(m->o1);
    o2.push_back(m->o2);
}
