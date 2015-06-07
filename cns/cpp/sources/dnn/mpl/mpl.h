#pragma once

#include <dnn/core.h>
#include <dnn/util/matrix.h>
#include <dnn/util/json.h>
#include <dnn/util/time_series.h>
#include <dnn/protos/generated.pb.h>
#include <dnn/io/serialize.h>

namespace dnn {


/*@GENERATE_PROTO@*/
struct MatchingPursuitConfig : public Serializable<Protos::MatchingPursuitConfig> {
	MatchingPursuitConfig() : 
	  threshold(0.1)
	, learn_iterations(100)
	, jobs(4)
	, learning_rate(1.0)
	, filters_num(100)
	, filter_size(100) 
	, learn(true)
	, continue_learning(false)
	, batch_size(1000)
	, seed(-1)
	, noise_sd(0.0)
	{}

	int seed;
	double threshold;
	size_t learn_iterations;
	size_t jobs;
	double learning_rate;
	size_t filters_num;
	size_t filter_size;
	bool learn;
	bool continue_learning;
	size_t batch_size;
	double noise_sd;

	void serial_process() {
        begin() <<
					"threshold: " << threshold << ", " <<
					"learn_iterations: " << learn_iterations << ", " <<
					"jobs: " << jobs << ", " <<
					"learning_rate: " << learning_rate << ", " <<
					"filters_num: " << filters_num << ", " <<
					"filter_size: " << filter_size << ", " <<
					"learn: " << learn << ", " <<
					"continue_learning: " << continue_learning <<
					"batch_size: " << batch_size <<
					"seed: " << seed <<
					"noise_sd: " << noise_sd
				<< Self::end;
	}


 //    Document serializeToJson() {
 //    	Document d;
 //    	d.SetObject();
 //    	d.AddMember("learn", learn, d.GetAllocator());
 //    	d.AddMember("continue_learning", continue_learning, d.GetAllocator());
	// 	d.AddMember("threshold", threshold, d.GetAllocator());
	//     d.AddMember("learn_iterations", learn_iterations, d.GetAllocator());
	//     d.AddMember("jobs", jobs, d.GetAllocator());
	//     d.AddMember("learning_rate", learning_rate, d.GetAllocator());
	//     d.AddMember("filters_num", filters_num, d.GetAllocator());
	//     d.AddMember("filter_size", filter_size, d.GetAllocator());
	//     return d;
	// }
	// void deserializeFromJson(Document &d) {
	// 	if(Json::checkVal(d, "learn")) learn = Json::getBoolVal(d, "learn");
	// 	if(Json::checkVal(d, "continue_learning")) continue_learning = Json::getBoolVal(d, "continue_learning");
	// 	if(Json::checkVal(d, "threshold")) threshold = Json::getDoubleVal(d, "threshold");
	// 	if(Json::checkVal(d, "learn_iterations")) learn_iterations = Json::getUintVal(d, "learn_iterations");
	// 	if(Json::checkVal(d, "jobs")) jobs = Json::getUintVal(d, "jobs");
	// 	if(Json::checkVal(d, "learning_rate")) learning_rate = Json::getDoubleVal(d, "learning_rate");
	// 	if(Json::checkVal(d, "filters_num")) filters_num = Json::getUintVal(d, "filters_num");
	// 	if(Json::checkVal(d, "filter_size")) filter_size = Json::getUintVal(d, "filter_size");
	// }
};

/*@GENERATE_PROTO@*/
struct FilterMatch : public Serializable<Protos::FilterMatch>  {
	FilterMatch() : fi(0), s(0.0), t(0) {}
	FilterMatch(size_t _fi, double _s, double _t) : fi(_fi), s(_s), t(_t) {}
	void serial_process() {
		begin() << "fi: " << fi << ", " << "s: " << s << ", " << "t: " << t << Self::end;
	}

	size_t fi;
	double s;
	size_t t;
};



class MatchingPursuit {
public:	
	MatchingPursuit(const MatchingPursuitConfig &_c) : c(_c) {
		if(c.seed < 0) {
			std::srand ( unsigned ( std::time(0) ) );
		} else {
			std::srand ( c.seed );
		}		
		filter.allocate(c.filters_num, c.filter_size);
		for(size_t i=0; i<c.filters_num; ++i) {
			double acc = 0.0;
	        for(size_t j=0; j<c.filter_size; ++j) {
	        	filter(i, j) = getNorm();
	        	acc += filter(i, j) * filter(i, j);		        	
	        }
	        double n = sqrt(acc);
	        for(size_t j=0; j<c.filter_size; ++j) {
	        	filter(i, j) = filter(i, j)/n;
	        }
	    }
	}

	

	struct SubSeqRet {
		SubSeqRet() {}
		
		vector<FilterMatch> matches;
		DoubleMatrix dfilter;
		
		vector<double> s;
		vector<size_t> winners_id;
	};
	
	vector<double> restore(const vector<FilterMatch> &matches) {
		TimeSeries ts;
		size_t max_t=0;
		for(auto &m: matches) {
			max_t = std::max(max_t, (size_t)m.t);
		}
		
		vector<double> restored;
		restored.resize(max_t + filter.ncol());
		_restore(matches, restored);
		return restored;
	}
	void _restore(const vector<FilterMatch> &matches, vector<double> &restored, size_t from=0) const {
		std::default_random_engine generator;
		std::normal_distribution<double> distribution(0.0, c.noise_sd);

		for(auto &v: restored) v = 0.0;
		for(auto &m: matches) {
			for(size_t i=0; i<filter.ncol(); ++i) {
				restored[m.t + i - from] += m.s * filter(m.fi, i) + distribution(generator);
			}
		}
	}
	static SubSeqRet runOnSubSeq(const MatchingPursuit &self, const TimeSeries &ts, size_t dim, size_t from, size_t to) {
		SubSeqRet r;
		if(self.c.learn) {
			r.dfilter.allocate(self.filter.nrow(), self.filter.ncol());
			r.dfilter.fill(0.0);
		}
		vector<double> x;
		for(size_t i=from; i<to; ++i) {
			if(i >= ts.data[dim].values.size()) {
				throw dnnException() << "Trying to get value out of input data: " << dim << ":" << i << "\n";
			}
			x.push_back(ts.data[dim].values[i]);
		}

		for(size_t ti=0; ti<(x.size()-self.filter.ncol()); ++ti) {
			size_t ti_f = ti+self.filter.ncol();
			for(size_t i=0; i<self.c.learn_iterations; ++i) {			
				double max_s = -100;
				size_t max_fi = 0;

				for(size_t fi=0; fi<self.filter.nrow(); ++fi) {
					double s_f=0;
					for(size_t xi=ti; xi<ti_f; ++xi) {
						s_f += x[xi] * self.filter(fi, xi-ti);					
					}

					if (max_s<s_f) {
						max_s = s_f;
						max_fi = fi;
					}
				}
				if(max_s>=self.c.threshold) {
					FilterMatch m(max_fi, max_s, ti+from);

					for(size_t xi=ti; xi<ti_f; ++xi) {
						x[xi] -= m.s * self.filter(m.fi, xi-ti);						
					}
					r.matches.push_back(m);
				} else {
					break;
				}
			}
		}	
		if( (self.c.learn) && (r.matches.size()>0) ) {
			vector<double> restored;
			restored.resize(x.size());
			self._restore(r.matches, restored, from);

			for(auto &m: r.matches) {
				for(size_t i=0; i<self.filter.ncol(); ++i) {
					double delta = ts.data[dim].values[m.t + i] - restored[m.t + i - from];
					r.dfilter(m.fi, i) += m.s * delta;
				}
			}
		}
		return r;
	}
	
	struct MPLReturn {
		MPLReturn() {}
		vector<FilterMatch> matches;
	};

	MPLReturn run(const TimeSeries &ts, const size_t dim) {
		MPLReturn runret;
		for(size_t bi=0; bi<ts.data[dim].values.size(); bi+=c.batch_size) { 
			vector<FilterMatch> matches;
			vector<IndexSlice> slices = dispatchOnThreads(
				std::min(ts.data[dim].values.size()-bi, c.batch_size), c.jobs
			);
			vector<std::future<SubSeqRet>> futures;
			for(auto &slice: slices) {
				futures.push_back(
					std::async(
						std::launch::async,
						runOnSubSeq, 
						std::cref(*this), 
						std::cref(ts), 
						dim, 
						bi+slice.from, 
						bi+slice.to
					)
				);
				//cout << "Running worker on slice " << bi+slice.from << ": " << bi+slice.to << "\n";
			}
			
			vector<SubSeqRet> rets;
			for(auto &fret: futures) {
				SubSeqRet ret = fret.get();
				//cout << "done\n";
				for(auto &m: ret.matches) {
					runret.matches.push_back(m);
				}
				rets.push_back(ret);				
			}
			if(c.learn) {
				for(auto &ret : rets) {
					for(size_t i=0; i<filter.nrow(); ++i) {
						double acc = 0.0;
						for(size_t j=0; j<filter.ncol(); ++j) {
							filter(i, j) += c.learning_rate * ret.dfilter(i, j);
							acc += filter(i, j) * filter(i, j);
						}
						double n = sqrt(acc);
						for(size_t j=0; j<filter.ncol(); ++j) {
							filter(i, j) = filter(i, j)/n;
						}
					}
				}
			}
		}
		return runret;
	}
	const DoubleMatrix& getFilter() {
		return filter;
	}

	void setFilter(const DoubleMatrix &m) {
		if( (m.nrow() != c.filters_num) || (m.ncol() != c.filter_size)) {
			throw dnnException() << "Got inappropriate to config matrix: need " << c.filters_num << ":" << c.filter_size << " size \n";
		}
		filter = m;

		filter.norm();
	}
protected:
	DoubleMatrix filter;
	MatchingPursuitConfig c;
};


}
