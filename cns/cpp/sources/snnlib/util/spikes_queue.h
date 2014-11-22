#pragma once

#include <snnlib/base.h>
#include <snnlib/util/spinlock.h>

bool compareSynSpike (const SynSpike& s1, const SynSpike& s2);

static int __test_global_id_sq = 0;

class SpikesQueue : public Printable {
public:
    SpikesQueue() {
        pos = l.begin();
        pthread_spin_init(&lock, 0);
    }
    void push_and_sort(const vector<SynSpike> &v);

    const inline void asyncPush(const SynSpike &s) {
        // if(print) {
        //     cout << __id <<  " asyncPush =============================\n";
        //     cout << *this << "\n";
        //     cout << "pushing " << s << "\n";
        // }
        list<SynSpike>::reverse_iterator it = l.rbegin();
        while(it != l.rend()) {
            if(s.t >= it->t) {
                break;
            }
            it++;
        }
        // if(print) {
        //     if(it.base() != l.end()) {
        //         cout << "pushing at place before " << *(it.base()) << "\n";
        //     } else {
        //         cout << "pushing at place before end\n";
        //     }
        // }
        pthread_spin_lock(&lock);
        l.insert(it.base(), s);
        if(pos == l.end()) {
            pos = it.base();
            pos--;               
            // if(print) {
            //     cout << "update for end pos\n";
            // }
        }

        if(s.t < pos->t) {
            pos = it.base();
            pos--;               
            // if(print) {
            //     cout << "update for pos\n";
            // }
        }
        pthread_spin_unlock(&lock);
        
        // if(print) {
        //     if(pos != l.end()) {
        //         cout << "now we are on " << *pos << "\n";
        //     } else {
        //         cout << "now we are on end\n";
        //     }
        //     cout << *this;
        //     cout << "=============================================\n";
        // }
    }
    inline const SynSpike* getSpike(const double &t) {
        // if(print) {
        //     cout << __id << " getSpike =============================\n";
        //     cout << *this << "\n";
        //     cout << "Seaching spike at " << t << "\n";
        //     if(pos == l.end()) {
        //         cout << "we are on end\n";
        //     } else {
        //         cout << "we are on that spike: " << *pos << "\n";    
        //     }
        // }

        list<SynSpike>::iterator it(pos);
        while(it != l.end()) {
            // if(print ) {
            //     cout << "considering " << *it << " (" << it->t << "<=" << t <<")\n";
            // }
            if(it->t > t) break; 
            if(it->t <= t) {
                // if(print) cout << "found " << *it << " and return it\n";
                pthread_spin_lock(&lock);
                pos++;
                pthread_spin_unlock(&lock);
                // if(print) {
                //     if(pos == l.end()) {
                //         cout << "and now we are on end\n";
                //     } else {
                //         cout << "and now we are on that spike: " << *pos << "\n";    
                //     }
                // }
                SynSpike *sp = &*it;
                
                return sp;
            }
            it++;
        }
        // if(print) { 
        //     cout <<"found nothing ";
        //     if(pos == l.end()) {
        //         cout << "| we are on end\n";
        //     } else {
        //         cout << "| we are on that spike: " << *pos << "\n";    
        //     }
        // }
        return nullptr;
    }
    void print(std::ostream& str) const {
        str << "SpikesQueue: ";
        for(auto it=l.begin(); it != l.end(); it++) {
            str << *it << ", ";
        }
        str << "\n";
    }
private:
    list<SynSpike> l;
    list<SynSpike>::iterator pos;
    pthread_spinlock_t lock;
};
