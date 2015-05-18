#pragma once

#include <dnn/core.h>
#include <dnn/util/matrix.h>
#include <dnn/util/json.h>
#include <dnn/util/time_series.h>

namespace dnn {

struct MPLConfig {
	MPLConfig() : 
	  threshold(0.1)
	, learn_iterations(100)
	, jobs(4)
	, learning_rate(1.0)
	, filters_num(100)
	, filter_size(100) 
	, learn(true)
	, continue_learning(false)
	{}

	double threshold;
	size_t learn_iterations;
	size_t jobs;
	double learning_rate;
	size_t filters_num;
	size_t filter_size;
	bool learn;
	bool continue_learning;
    
    Document serializeToJson() {
    	Document d;
    	d.SetObject();
    	d.AddMember("learn", learn, d.GetAllocator());
    	d.AddMember("continue_learning", continue_learning, d.GetAllocator());
		d.AddMember("threshold", threshold, d.GetAllocator());
	    d.AddMember("learn_iterations", learn_iterations, d.GetAllocator());
	    d.AddMember("jobs", jobs, d.GetAllocator());
	    d.AddMember("learning_rate", learning_rate, d.GetAllocator());
	    d.AddMember("filters_num", filters_num, d.GetAllocator());
	    d.AddMember("filter_size", filter_size, d.GetAllocator());
	    return d;
	}
	void deserializeFromJson(Document &d) {
		if(Json::checkVal(d, "learn")) learn = Json::getBoolVal(d, "learn");
		if(Json::checkVal(d, "continue_learning")) continue_learning = Json::getBoolVal(d, "continue_learning");
		if(Json::checkVal(d, "threshold")) threshold = Json::getDoubleVal(d, "threshold");
		if(Json::checkVal(d, "learn_iterations")) learn_iterations = Json::getUintVal(d, "learn_iterations");
		if(Json::checkVal(d, "jobs")) jobs = Json::getUintVal(d, "jobs");
		if(Json::checkVal(d, "learning_rate")) learning_rate = Json::getDoubleVal(d, "learning_rate");
		if(Json::checkVal(d, "filters_num")) filters_num = Json::getUintVal(d, "filters_num");
		if(Json::checkVal(d, "filter_size")) filter_size = Json::getUintVal(d, "filter_size");
	}
};


class MPL {
public:	
	struct FilterMatch {
		FilterMatch(size_t _fi, size_t _s, double _t) : fi(_fi), s(_s), t(_t) {}
		size_t fi;
		size_t s;
		double t;
	};

	struct SubSeqRet {
		vector<FilterMatch> matches;
		DoubleMatrix dfilter;
	};

	static SubSeqRet runOnSubSeq(const TimeSeries &ts, const MPLConfig &c, const DoubleMatrix &filter, size_t dim, size_t from) {
		SubSeqRet r;
		vector<double> x;
		for(size_t i=from; i<(from+filter.ncol()); ++i) {
			x.push_back(ts.data[dim].values[i]);
		}
		{
			DoubleMatrix xser(x); 
			ofstream of("./xser.pb");
			Stream(of, Stream::Binary).writeObject(&xser);
		}

		vector<double> s;
		vector<size_t> winners_id;
		for(size_t i=0; i<c.learn_iterations; ++i) {			
			double max_s = -100;
			size_t max_fi = 0;

			for(size_t fi=0; fi<filter.ncol(); ++fi) {
				double s_f=0;
				for(size_t xi=0; xi<x.size(); ++xi) {
					s_f += x[i] * filter.getElement(xi, fi);					
				}
				if (max_s<s_f) {
					max_s = s_f;
					max_fi = fi;
				}
			}
			
			cout << "s: " << max_s << ", " << "fi: " << max_fi << "\n";
			s.push_back(max_s);
			winners_id.push_back(max_fi);

			double res_acc=0;
			for(size_t xi=0; xi<x.size(); ++xi) {
				x[xi] -= max_s * filter.getElement(xi, max_fi);
				res_acc += x[xi]*x[xi];				
			}
			cout << "Residual: " <<  sqrt(res_acc) << "\n";
			if(max_s>=c.threshold) {
				r.matches.push_back(FilterMatch(max_fi, max_s, from));
			}
		}
		return r;
	}

	static vector<FilterMatch> run(const TimeSeries &ts, const MPLConfig &c, DoubleMatrix &filter) {
		if((!c.continue_learning)&&(c.learn)) {
			filter.allocate(c.filters_num, c.filter_size);
			for(size_t i=0; i<c.filters_num; ++i) {
				double acc = 0.0;
		        for(size_t j=0; j<c.filter_size; ++j) {
		        	filter.setElement(i, j, getNorm());
		        	acc += filter.getElement(i, j) * filter.getElement(i, j);		        	
		        }
		        double n = sqrt(acc);
		        for(size_t j=0; j<c.filter_size; ++j) {
		        	filter.setElement(i, j, filter.getElement(i, j)/n);
		        }
		    }

		}
		runOnSubSeq(ts, c, filter, 0, 0);

		vector<FilterMatch> matches;
		return matches;
	}
};


}