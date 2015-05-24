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

	struct FilterMatch {
		FilterMatch(size_t _fi, double _s, double _t) : fi(_fi), s(_s), t(_t) {}
		size_t fi;
		double s;
		size_t t;
	};

	struct SubSeqRet {
		SubSeqRet() : accum_error(0.0) {}
		
		vector<FilterMatch> matches;
		DoubleMatrix dfilter;
		
		vector<double> s;
		vector<size_t> winners_id;
		double accum_error;
	};
	
	vector<double> restore(const vector<FilterMatch> matches) {
		TimeSeries ts;
		size_t max_t=0;
		for(auto &m: matches) {
			max_t = std::max(max_t, (size_t)m.t);
		}
		
		vector<double> restored;
		restored.resize(max_t + filter.ncol());
		for(auto &v: restored) v = 0.0;
		
		for(auto &m: matches) {
			for(size_t i=0; i<filter.ncol(); ++i) {
				restored[m.t + i] += m.s * filter(m.fi, i) + c.noise_sd * getNorm();
			}
		}
		return restored;
	}

	static SubSeqRet runOnSubSeq(const MatchingPursuit &self, const TimeSeries &ts, size_t dim, size_t from, size_t to) {
		SubSeqRet r;
		if(self.c.learn) {
			r.dfilter.allocate(self.filter.nrow(), self.filter.ncol());
			r.dfilter.fill(0.0);
		}
		vector<double> x;
		for(size_t i=from; i<(to-self.filter.ncol()); ++i) {
			if(i >= ts.data[dim].values.size()) {
				throw dnnException() << "Trying to get value out of input data: " << dim << ":" << i << "\n";
			}
			x.push_back(ts.data[dim].values[i]);
		}

		for(size_t ti=0; ti<x.size(); ++ti) {
			// vector<double> x;
			// for(size_t i=ti; i<(ti+self.filter.ncol()); ++i) {
			// 	if(i >= ts.data[dim].values.size()) {
			// 		throw dnnException() << "Trying to get value out of input data: " << dim << ":" << i << "\n";
			// 	}
			// 	x.push_back(ts.data[dim].values[i]);
			// }
			// {
			// 	DoubleMatrix mm(x);
			// 	char buf[100];
			// 	sprintf(buf, "%zu_x%zu.pb", self.ep, ti);
			// 	ofstream of(buf);
			// 	Stream(of, Stream::Binary).writeObject(&mm);
			// }

			// vector<double> s;
			// vector<size_t> winners_id;

			vector<FilterMatch> matches;
			for(size_t i=0; i<self.c.learn_iterations; ++i) {			
				double max_s = -100;
				size_t max_fi = 0;

				for(size_t fi=0; fi<self.filter.nrow(); ++fi) {
					double s_f=0;
					for(size_t xi=ti; xi<(ti+self.filter.ncol()); ++xi) {
					//for(size_t xi=0; xi<x.size(); ++xi) {
						//s_f += x[xi] * self.filter(fi, xi);					
						s_f += x[xi] * self.filter(fi, xi-ti);					
					}

					if (max_s<s_f) {
						max_s = s_f;
						max_fi = fi;
					}
				}
				if(max_s>=self.c.threshold) {
					FilterMatch m(max_fi, max_s, ti);

					for(size_t xi=ti; xi<(ti+self.filter.ncol()); ++xi) {
					//for(size_t xi=0; xi<x.size(); ++xi) {
						//x[xi] -= m.s * self.filter(m.fi, xi);
						x[xi] -= m.s * self.filter(m.fi, xi-ti);						
					}
					matches.push_back(m);
				} else {
					break;
				}
			}
			r.matches.insert(r.matches.end(), matches.begin(), matches.end());
			// if ((self.c.learn) && (matches.size() > 0)) {
			// 	vector<double> x_des;
			// 	x_des.resize(x.size());
			// 	for(size_t mi=0; mi<matches.size(); ++mi) {
			// 		for(size_t xi=0; xi<x.size(); ++xi) {
			// 			if(mi == 0) x_des[xi] = 0.0;

			// 			x_des[xi] += matches[mi].s * self.filter(matches[mi].fi, xi);
			// 		}
			// 	}

			// 	vector<double> deltas;
			// 	deltas.resize(x.size());
			// 	for(size_t xi=0; xi<x.size(); ++xi) {
			// 		const double &x_start = ts.data[dim].values[ti+xi];
			// 		deltas[xi] = x_start - x_des[xi];
					
			// 		r.accum_error += deltas[xi] * deltas[xi];
			// 	}
			// 	for(auto &m: matches) {
			// 		for(size_t xi=0; xi<x.size(); ++xi) {
			// 			r.dfilter(m.fi, xi) += m.s * deltas[xi];  
			// 		}
			// 	}
				
				
				// {
				// 	DoubleMatrix mm(x_des);
				// 	char buf[100];
				// 	sprintf(buf, "%zu_xdes%zu.pb", self.ep, ti);
				// 	ofstream of(buf);
				// 	Stream(of, Stream::Binary).writeObject(&mm);
				// }
				// {
				// 	DoubleMatrix mm(deltas);
				// 	char buf[100];
				// 	sprintf(buf, "%zu_deltas%zu.pb", self.ep, ti);
				// 	ofstream of(buf);
				// 	Stream(of, Stream::Binary).writeObject(&mm);
				// }
				// {
				// 	DoubleMatrix mm(s);
				// 	char buf[100];
				// 	sprintf(buf, "%zu_s%zu.pb", self.ep, ti);
				// 	ofstream of(buf);
				// 	Stream(of, Stream::Binary).writeObject(&mm);
				// }
				// {
				// 	vector<double> www;
				// 	for(auto &wid: winners_id) {
				// 		www.push_back(wid);
				// 	}
				// 	DoubleMatrix mm(www);
				// 	char buf[100];
				// 	sprintf(buf, "%zu_winners_id%zu.pb", self.ep, ti);
				// 	ofstream of(buf);
				// 	Stream(of, Stream::Binary).writeObject(&mm);
				// }
				
			// }
			
		}	

		return r;
	}
	
	struct MPLReturn {
		MPLReturn() : accum_error(0.0) {}
		vector<FilterMatch> matches;
		double accum_error;
		vector<double> restored;
	};

	MPLReturn run(const TimeSeries &ts, const size_t dim) {
		MPLReturn ret;
		for(size_t bi=0; bi<ts.data[dim].values.size(); bi+=c.batch_size) { 
			vector<FilterMatch> matches;
			vector<IndexSlice> slices = dispatchOnThreads(c.batch_size, c.jobs);
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
				cout << "Running worker on slice " << bi+slice.from << ": " << bi+slice.to << "\n";
			}
			
			for(auto &fret: futures) {
				SubSeqRet ret = fret.get();
				for(auto &m: ret.matches) {
					matches.push_back(m);
				}
				//accum_error += ret.accum_error;
			}
				// {
				// 	char buf[100];
				// 	sprintf(buf, "%zu_dfilter.pb", ep);
				// 	ofstream o(buf);
				// 	Stream(o, Stream::Binary).writeObject(&ret.dfilter);
				// }
			if(c.learn) {
				double accum_error = 0;

				vector<double> restored;
				restored.resize(c.batch_size);
				for(auto &r: restored) r = 0.0;

				for(auto &m: matches) {
					for(size_t i=0; i<filter.ncol(); ++i) {
						restored[m.t + i - bi] = m.s * filter(m.fi, i);
					}
				}
				for(auto &m: matches) {
					double acc = 0.0;
					for(size_t i=0; i<filter.ncol(); ++i) {
						double delta = ts.data[dim].values[m.t+i] - restored[m.t + i - bi];
						filter(m.fi, i) += c.learning_rate * m.s * delta;
						
						accum_error += delta * delta;
						acc += filter(m.fi, i);
					}
					double n = sqrt(acc);
					for(size_t i=0; i<filter.ncol(); ++i) {
						filter(m.fi, i) = filter(m.fi, i)/n;
					}
				}
				
				ret.restored.insert(ret.restored.end(), restored.begin(), restored.end());
				ret.accum_error += accum_error;

				// for(size_t i=0; i<filter.nrow(); ++i) {
				// 	double acc = 0.0;
				// 	for(size_t j=0; j<filter.ncol(); ++j) {
				// 		filter(i, j) += c.learning_rate * ret.dfilter(i, j);
				// 		acc += filter(i, j) * filter(i, j);
				// 	}
				// 	double n = sqrt(acc);
				// 	for(size_t j=0; j<filter.ncol(); ++j) {
				// 		filter(i, j) = filter(i, j)/n;
				// 	}
				// }
				

			}
			ret.matches.insert(ret.matches.end(), matches.begin(), matches.end());
		}
		return ret;
	}
	const DoubleMatrix& getFilter() {
		return filter;
	}
	void setFilter(const DoubleMatrix &m) {
		filter = m;
	}
protected:

	DoubleMatrix filter;
	MatchingPursuitConfig c;
};


}
