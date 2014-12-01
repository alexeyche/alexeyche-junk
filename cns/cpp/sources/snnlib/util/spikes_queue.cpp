
#include "spikes_queue.h"


bool compareSynSpike (const SynSpike& s1, const SynSpike& s2) {
    if (s1.t < s2.t) return true;
    return false;
}


void SpikesQueue::push_and_sort(vector<SynSpike> v) {
    for(auto it=v.begin(); it != v.end(); ++it) {
        l.push_back(*it);
    }
    l.sort(compareSynSpike);
    pos = l.begin();
}
