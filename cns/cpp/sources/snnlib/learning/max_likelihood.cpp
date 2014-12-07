

#include "max_likelihood.h"

MaxLikelihoodStat::MaxLikelihoodStat(MaxLikelihood *m) : Serializable<Protos::MaxLikelihoodStat>(EMaxLikelihoodStat) {
	for(size_t ei=0; ei < m->eligibility_trace.size(); ei++) {
		eligibility_trace.push_back(vector<double>());
	}
}
void MaxLikelihoodStat::collect(MaxLikelihood *m) {
    if((eligibility_trace.size()>1)&&(eligibility_trace[0].size()>STAT_COLLECT_LIMIT)) return;
    for(size_t syn_i=0; syn_i < m->eligibility_trace.size(); syn_i++) {
    	eligibility_trace[syn_i].push_back(m->eligibility_trace[syn_i]);
    }
}