#pragma once

#include <snnlib/base.h>
#include <snnlib/util/spinlock.h>

bool compareSynSpike (const SynSpike& s1, const SynSpike& s2);

class SpikesQueue : public Printable {
public:
    SpikesQueue() {
        pos = l.begin();
    }
    void push_and_sort(const vector<SynSpike> &v);

    const inline void asyncPush(const SynSpike &s) {
        list<SynSpike>::iterator it(pos);
        cout << "asyncPush =============================\n";
        cout << *this << "\n";
        cout << "pushing " << s << "\n";
        while(it != l.end()) {
            if(s.t <= it->t) {
                break;
            }
            it++;
        }
        if(it != l.end()) {
            cout << "pushing at place before " << *it << "\n";
        } else {
            cout << "pushing at end\n";
        }
        sp_lock.lock();
        l.insert(it, s);
        pos--;
        sp_lock.unlock();
        cout << *this;
        cout << "=============================================\n";
    }
    inline const SynSpike* getSpike(const double &t) {
        cout << "getSpike =============================\n";
        cout << *this << "\n";
        cout << "Seaching spike at " << t << "\n";
        while(pos != l.end()) {
            cout << pos->t << " < " << t << "\n";
            if(pos->t > t) break;
            const SynSpike* sp = &*pos;
            cout << "found " << *sp << "\n";
            cout << "=============================================\n";
            pos++;
            if(pos != l.end()) {
                cout << "inc pos and now we on " << *pos << "\n";
            } else {
                cout << "inc pos and now we on end\n";
            }
            return sp;
        }
        // cout << "found nothing\n";
        // cout << "=============================================\n";
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
    SpinLock sp_lock;
};
