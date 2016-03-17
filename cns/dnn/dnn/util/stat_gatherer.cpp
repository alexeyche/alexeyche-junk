#include "stat_gatherer.h"

#include <dnn/util/serial/bin_serial.h>

#include <fstream>

namespace NDnn {
	
	TStatGatherer::TStatGatherer() {
	}

	TStatGatherer::TStatGatherer(const TStatGatherer& other) {
		(*this) = other;	
	}

	TStatGatherer& TStatGatherer::operator=(const TStatGatherer& other) {
		if (this != &other) {
			Stats = other.Stats;
			ActiveStats.clear();
			for (auto& stat: Stats) {
				ActiveStats.push_back(&stat);
			}
		}
		return *this;
	}

	void TStatGatherer::ListenStat(const TString& name, const double& v, ui32 from, ui32 to) {
		Stats.emplace_back(name, v, from, to);
		ActiveStats.push_back(&Stats.back());
	}

	void TStatGatherer::Collect(const TTime& t) {
		auto statIt = ActiveStats.begin();
		while (statIt != ActiveStats.end()) {
			L_DEBUG << t.T << " ? " << statIt->Get()->GetTo();
			
			if (t.T > statIt->Get()->GetTo()) {

				ActiveStats.erase(statIt);
				continue;
			}
			if (t.T >= statIt->Get()->GetFrom()) {
				statIt->Get()->Collect();
			}
		}
	}

	void TStatGatherer::SaveStat(const TString& fname) {
		std::ofstream output(fname, std::ios::binary);
	    TBinSerial serial(output);
	    for (auto& stat: Stats) {
	    	serial.WriteObject<TStatistics>(stat.GetMutStatistics());
	    }
	}


} // namespace NDnn