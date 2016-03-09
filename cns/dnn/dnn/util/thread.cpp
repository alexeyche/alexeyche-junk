#include "thread.h"

#include <cmath>

namespace NDnn {

    TVector<TIndexSlice> DispatchOnThreads(ui32 elements_size, ui32 jobs) {
        TVector<TIndexSlice> out;
        for(ui32 ji=0; ji < jobs; ji++) {
            int el_per_thread;
            if(fabs(fmod(elements_size, jobs)) < 0.000001) {
                el_per_thread = elements_size / jobs;
            } else {
                el_per_thread = (elements_size + jobs - 1) / jobs;
            }

            ui32 first = std::min( ji    * el_per_thread, elements_size );
            ui32 last  = std::min( (ji+1) * el_per_thread, elements_size );
            if(first != last) {
                out.push_back( TIndexSlice(first, last) );
            }
        }
        return out;
    }

} // namespace NDnn