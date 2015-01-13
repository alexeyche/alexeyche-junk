
#include "bcm_rule.h"


BCMRuleStat::BCMRuleStat(BCMRule *m) : Serializable<Protos::BCMRuleStat>(EBCMRuleStat) {
	for(size_t i=0; i < m->x.size(); i++) {
		x.push_back(vector<double>());
	}
}

void BCMRuleStat::collect(BCMRule *m) {
	if(y.size() > STAT_COLLECT_LIMIT) return;
	for(size_t syn_i=0; syn_i < m->x.size(); syn_i++) {
    	x[syn_i].push_back(m->x[syn_i]);
    }
    y.push_back(m->y);
    p_acc.push_back(m->p_acc);
}
