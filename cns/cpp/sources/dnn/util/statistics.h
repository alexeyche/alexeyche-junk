#pragma once

#include <dnn/protos/generated.pb.h>
#include <dnn/io/serialize.h>

namespace dnn {

/*@GENERATE_PROTO@*/
struct Stat : public Serializable<Protos::Stat> {
	Stat() : low_lim(-1), high_lim(-1), __counter(0) {} 
	
	void serial_process() {
		begin() << "name: " << name << ", "
				<< "vals: " << vals << ", "
				<< "low_lim: " << low_lim << ", "
				<< "high_lim: " << high_lim << Self::end;
	}

	void add(const double &s) {
		if( ((low_lim>0)&&(__counter<low_lim)) || ((high_lim>0)&&(__counter>=high_lim)) ) { __counter++; return; }
		vals.push_back(s);
		__counter++;
	}

	size_t __counter;

	int low_lim;
	int high_lim;

	string name;
	vector<double> vals;
};

struct StatInfo : public Serializable<Protos::Stat> {	
	void serial_process() {
		begin() << "name: " 	<< name 	<< ", "
		 	    << "stat_num: " << stat_num << Self::end;
	}

	string name;
	size_t stat_num;
};


class Statistics : public SerializableBase {
public:
	Statistics() : low_lim(-1), high_lim(-1), on(false) {
		const char* stat_limit_str = std::getenv("STAT_LIMIT");
		if(stat_limit_str) {
			vector<string> spl = split(stat_limit_str, ':');
			if(spl.size() == 2) {
				low_lim  = std::stof(spl[0]);
				high_lim = std::stof(spl[1]);
			} else {
				high_lim = std::stof(stat_limit_str);
			}
		}
	}
	const string name() const {
		return "Statistics";
	}

	void serial_process() {
		begin() << "info: " << info << ", ";
		for(auto it=stats.begin(); it != stats.end(); ++it){
			(*this) << "stat: " << it->second << ", ";
		}
		(*this) << Self::end;
	}

	void turnOn() {
		on = true;
	}
	
	void add(const string &name, const double &v) {
		if(!on) return;
		if(stats.find(name) == stats.end()) {
			Stat s;
			s.name = name;
			s.low_lim = low_lim;
			s.high_lim = high_lim;
			stats[name] = s;
		}
		stats[name].add(v);
	}

	StatInfo info;
	map<string, Stat> stats;
	int low_lim;
	int high_lim;
	bool on;
};


}