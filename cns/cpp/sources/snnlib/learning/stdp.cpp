
#include "stdp.h"


StdpStat::StdpStat(Stdp *m) : Serializable<Protos::StdpStat>(EStdpStat) {
	for(size_t i=0; i < m->x_trace.size(); i++) {
		x_trace.push_back(vector<double>());
	}
}

void StdpStat::collect(Stdp *m) {
	if(y_trace.size() > STAT_COLLECT_LIMIT) return;
	for(size_t syn_i=0; syn_i < m->x_trace.size(); syn_i++) {
    	x_trace[syn_i].push_back(m->x_trace[syn_i]);
    }
    y_trace.push_back(m->y_trace);
}
