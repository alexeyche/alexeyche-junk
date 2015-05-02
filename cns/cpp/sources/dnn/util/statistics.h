#pragma once

#include <dnn/protos/generated.pb.h>
#include <dnn/io/serialize.h>
#include <regex>

namespace dnn {

/*@GENERATE_PROTO@*/
struct Stat : public Serializable<Protos::Stat> {
	Stat() : low_lim(-1), high_lim(-1), __counter(0) {}

	void serial_process() {
		begin() << "values: " << values << ", "
		        << "low_lim: " << low_lim << ", "
		        << "high_lim: " << high_lim << Self::end;
	}

	void add(const double &s) {
		if ( ((low_lim > 0) && (__counter < low_lim)) || ((high_lim > 0) && (__counter >= high_lim)) ) { __counter++; return; }
		values.push_back(s);
		__counter++;
	}

	size_t __counter;

	int low_lim;
	int high_lim;

	vector<double> values;
};

/*@GENERATE_PROTO@*/
struct StatisticsInfo : public Serializable<Protos::StatisticsInfo> {
	void serial_process() {
		begin() << "stat_names: " << stat_names << ", "
		        << "low_lim: "    << low_lim    << ", "
		        << "high_lim: "   << high_lim << Self::end;
	}
	int low_lim;
	int high_lim;
	vector<string> stat_names;
};



struct stringLessThan : public std::binary_function< string, string, bool >
{
	stringLessThan() 
	: e("^([0-9]+)", std::regex_constants::basic) {}

    bool operator()( const string &e1, const string &e2 ) const
    {
        std::smatch sm1, sm2; 
    	std::regex_match (e1,sm1,e);
    	std::regex_match (e2,sm2,e);    	
    	if((sm1.size() == 2)&&(sm2.size() == 2)) {
    		return std::stoi(sm1[1]) < std::stoi(sm2[1]);
    	}    	
        return e1 < e2;
    }
    std::regex e;
};

class Statistics : public SerializableBase {
public:
	Statistics() : low_lim(-1), high_lim(10000), _on(false) {
		const char* stat_limit_str = std::getenv("STAT_LIMIT");
		if (stat_limit_str) {
			vector<string> spl = split(stat_limit_str, ':');
			if (spl.size() == 2) {
				low_lim  = std::stoi(spl[0]);
				high_lim = std::stoi(spl[1]);
			} else {
				high_lim = std::stoi(stat_limit_str);
			}
		}
	}
	const string name() const {
		return "Statistics";
	}
	StatisticsInfo getInfo() {
		StatisticsInfo info;
		for (auto it = stats.begin(); it != stats.end(); ++it) {
			info.stat_names.push_back(it->first);
		}
		std::sort(info.stat_names.begin(), info.stat_names.end(), stringLessThan());
		info.low_lim = low_lim;
		info.high_lim = high_lim;
		return info;
	}
	void serial_process() {
		StatisticsInfo info;
		if (mode == ProcessingOutput) {
			info = getInfo();
		}

		begin() << "info: " << info << ", ";
		for (auto it = info.stat_names.begin(); it != info.stat_names.end(); ++it) {
			(*this) << "stat: " << stats[*it] << ", ";
		}
		(*this) << Self::end;
	}

	void turnOn() {
		_on = true;
	}

	const bool&	on() const {
		return _on;
	}
	inline void add(const string &name, size_t sub_ind, const double &v) {
		add(name + std::to_string(sub_ind), v);
	}
	inline void add(const string &name, const double &v) {
		if (!_on) return;
		if (stats.find(name) == stats.end()) {
			Stat s;
			s.low_lim = low_lim;
			s.high_lim = high_lim;
			stats[name] = s;
		}
		stats[name].add(v);
	}

	map<string, Stat>& getStats() {
		return stats;
	}
	
	map<string, Stat> stats;
	int low_lim;
	int high_lim;
	bool _on;
};


}