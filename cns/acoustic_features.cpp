#include <vector>
#include <stdexcept>
#include <cmath>
#include <iostream>
#include <random>
#include <chrono>
#include <thread>
#include <future>

using std::size_t;

struct TMatchingPursuitConfig { 
	int Seed = -1;
	double Threshold = 0.1;
	size_t LearnIterations = 100;
	size_t Jobs = 1;
	double LearningRate = 0.1;
	size_t FiltersNum = 100;
	size_t FilterSize = 200;
	bool Learn = true;
	bool ContinueLearning = false;
	size_t BatchSize = 1000;
	double NoiseSd = 0.0;
	double Momentum = 0.9;
};

struct TFilterMatch {
	TFilterMatch() {}
	TFilterMatch(size_t fi, double s, double t) : Fi(fi), S(s), T(t) {}
	
	size_t Fi = 0;
	double S = 0.0;
	size_t T = 0;
};

struct TIndexSlice {
    TIndexSlice(size_t from, size_t to) : From(from), To(to), Size(To-From) {}

    size_t From;
    size_t To;
    size_t Size;
};


std::vector<TIndexSlice> DispatchOnThreads(size_t elements_size, size_t jobs) {
    std::vector<TIndexSlice> out;
    double el_per_thread = static_cast<double>(elements_size) / jobs;

    for(size_t ji=0; ji < jobs; ji++) {
        size_t first = std::min( static_cast<size_t>(std::floor(ji * el_per_thread)), elements_size );
        size_t last  = std::min( static_cast<size_t>(std::floor((ji+1) * el_per_thread)), elements_size );
        
        out.push_back( TIndexSlice(first, last) );
    }
    return out;
}


class TMatchingPursuit {
private:
	TMatchingPursuitConfig Config;
	double* _Filters; 
	
	mutable std::mt19937 Generator;
	mutable std::normal_distribution<double> NormDistr;

	std::vector<std::vector<double>> DfilterMoment;
private:
	struct TSubSeqRet {
		std::vector<TFilterMatch> Matches;
		std::vector<std::vector<double>> Dfilter;

		std::vector<double> S;
		std::vector<double> Residual;
		std::vector<size_t> WinnersId;
		double MeanLoss = 0.0;
	};
	
public:
	double& Filter(size_t fnum, size_t fi) {
		return _Filters[fnum * Config.FilterSize + fi];
	}

	const double& Filter(size_t fnum, size_t fi) const {
		return _Filters[fnum * Config.FilterSize + fi];
	}

	TMatchingPursuit(const TMatchingPursuitConfig& config, double* filters)
		: Config(config)
		, _Filters(filters)
		, NormDistr(0.0, 1.0)
	{
		size_t seedNum = Config.Seed >= 0 ? Config.Seed : std::chrono::system_clock::now().time_since_epoch().count();
		Generator = std::mt19937(seedNum);
		
		for (size_t fnum=0; fnum < Config.FiltersNum; ++fnum) {
			double acc = 0.0;
			for (size_t fi=0; fi < Config.FilterSize; ++fi) {
	        	const double& v = Filter(fnum, fi);
	        	acc += v*v;
	        }
	        acc = std::sqrt(acc + 1e-10);

			for (size_t fi=0; fi < Config.FilterSize; ++fi) {
	        	Filter(fnum, fi) = Filter(fnum, fi)/acc;
	        	// std::cout << Filter(fnum, fi) << ", ";
			}
			// std::cout << "\n";
		}
		DfilterMoment.resize(Config.FiltersNum);
		for (auto& filter: DfilterMoment) {
			filter.resize(Config.FilterSize, 0.0);
		}
	}

	struct TReturn {
		std::vector<TFilterMatch> Matches;
		std::vector<double> Residual;
		double MeanLoss = 0.0;
	};

	std::vector<double> Restore(const TFilterMatch* matches, size_t matches_len) {
	    size_t max_t=0;
	    for (size_t mi=0; mi < matches_len; ++mi) {
	    	max_t = std::max(max_t, matches[mi].T);
	    }

	    std::vector<double> restored;
	    restored.resize(max_t + Config.FilterSize, 0.0);
	    RestoreImpl(matches, matches_len, restored);
	    return restored;
	}


	TSubSeqRet RunOnSubSeq(const std::vector<double>& ts, size_t from, size_t to) const {
		TSubSeqRet r;
	    if(Config.Learn) {
	        r.Dfilter.resize(Config.FiltersNum);
	        for (auto& f: r.Dfilter) {
	        	f.resize(Config.FilterSize, 0.0);
	        }
	    }
	    for(size_t i=from; i<to; ++i) {
	        r.Residual.push_back(ts.at(i));
	    }
	    std::cout << from << " " << to << "\n";
		// std::cout << r.Residual.size() << " " << Config.FilterSize << "\n";
	    for(size_t ti=0; ti<(r.Residual.size()-Config.FilterSize); ++ti) {
	    	size_t ti_f = ti+Config.FilterSize;

	        double x_denom = 0.0;
	        for(size_t xi=ti; xi<ti_f; ++xi) {
	            x_denom += r.Residual[xi]*r.Residual[xi];
	        }
	        x_denom = std::sqrt(x_denom+1e-10);
	        std::cout << "Denom: " << x_denom << "\n";
	        for(size_t i=0; i<Config.LearnIterations; ++i) {
	            double max_s = -100;
	            size_t max_fi = 0;

	            for(size_t fi=0; fi<Config.FiltersNum; ++fi) {
	                double s_f=0;
	                // std::cout << "Filter " << fi << "\n";
	                for(size_t xi=ti; xi<ti_f; ++xi) {
	                	// std::cout << r.Residual[xi] << "/" << x_denom << " * " << Filter[fi][xi-ti] << "\n";
	                    s_f += (r.Residual[xi]/x_denom) * Filter(fi, xi-ti);
	                }
	                // std::cout << "s_f == " << s_f << "\n"; 

	                if (std::fabs(max_s) < std::fabs(s_f)) {
	                    max_s = s_f;
	                    max_fi = fi;
	                }
	            }
	            // std::cout << "ti: " << ti << " " << max_s << " " << max_fi << "\n";
	            // double noise = distribution(generator);
	            double noise = 0.0;
	            if(max_s+noise>=Config.Threshold) {
	                // L_DEBUG << "noise: " << noise << ", max_s: " << max_s;
	                std::cout << "Got match " << max_fi << " " << max_s << " at " << ti + from << "\n";
	                TFilterMatch m(max_fi, max_s, ti+from);

	                for(size_t xi=ti; xi<ti_f; ++xi) {
	                    r.Residual[xi] -= x_denom * m.S * Filter(m.Fi, xi-ti);
	                }

	                r.Matches.push_back(m);
	            } else {
	                break;
	            }
	        }
	    }
	    if( (Config.Learn) && (r.Matches.size()>0) ) {
	        std::vector<double> restored;
	        restored.resize(r.Residual.size());
	        RestoreImpl(&r.Matches[0], r.Matches.size(), restored, from);
	        for(const auto& m: r.Matches) {
	            double x_denom = 0.0;
	            double y_denom = 0.0;
	            for(size_t i=0; i<Config.FilterSize; ++i) {
	                x_denom += ts[m.T + i] * ts[m.T + i];
	                y_denom += restored[m.T + i - from] * restored[m.T + i - from];
	            }
	            x_denom = std::sqrt(x_denom + 1e-10);
	            y_denom = std::sqrt(y_denom + 1e-10);
	            for(size_t i=0; i<Config.FilterSize; ++i) {
	                double delta = ts[m.T + i]/x_denom - restored[m.T + i - from]/y_denom;
	                // std::cout << delta << "\n";
	                // double delta = ts[m.T + i] - restored[m.T + i - from];
	                r.Dfilter[m.Fi][i] += m.S * delta;
	                r.MeanLoss += delta*delta;
	            }
	        }
	        r.MeanLoss = r.MeanLoss/r.Matches.size()/Config.FilterSize;
	    }
	    return r;
	}

	
    TReturn Run(const std::vector<double>& ts) {
		TReturn runret;
		size_t iter = 0;
	    for(size_t bi=0; bi<ts.size(); bi+=Config.BatchSize) {
	        iter += 1;
	        std::vector<TFilterMatch> matches;
	        std::vector<TIndexSlice> slices = DispatchOnThreads(std::min(ts.size()-bi, Config.BatchSize), Config.Jobs);
	        std::vector<std::future<TSubSeqRet>> futures;
	        for(auto& slice: slices) {
	        	if (slice.Size <= Config.FilterSize) {
	        		throw std::runtime_error("Size of slice per job less than filter size, consider to make filter size smaller or batch size bigger");
	        	}
	        	size_t ts_from = bi + slice.From;
	        	if (ts_from > 0) {
	        		ts_from -= Config.FilterSize;
	        	}
	            size_t ts_to = bi + slice.To;
	            futures.push_back(
	                std::async(
	                    std::launch::async,
	                    [&](const std::vector<double>& data, size_t from, size_t to) {
	                    	return RunOnSubSeq(data, from, to);
	                    },
	                    std::cref(ts),
	                    ts_from,
	                    ts_to
	                )
	            );
	            std::cout << "Running worker on slice " << ts_from << ": " << ts_to << "\n";
	        }

	        std::vector<TSubSeqRet> rets;
	        double mean_loss = 0.0;
	        for(auto& fret: futures) {
	            TSubSeqRet ret = fret.get();
	            std::cout << "Done\n";
            	mean_loss += ret.MeanLoss;
	            for(auto& m: ret.Matches) {
	                runret.Matches.push_back(m);
	            }
	            // std::cout << "Mean loss: " << ret.MeanLoss << "\n";
	            for(const auto& r: ret.Residual) {
	                runret.Residual.push_back(r);
	            }
	            rets.push_back(ret);
	        }

	        if(Config.Learn) {
	        	runret.MeanLoss += mean_loss/futures.size();
	            for (auto& ret: rets) {
	                for(size_t i=0; i<Config.FiltersNum; ++i) {
	                    double acc = 0.0;
	                    for (size_t j=0; j<Config.FilterSize; ++j) {
	                    	DfilterMoment[i][j] = DfilterMoment[i][j] * Config.Momentum + ret.Dfilter[i][j];
	                        Filter(i, j) += Config.LearningRate * DfilterMoment[i][j];
	                        acc += Filter(i, j) * Filter(i, j);
	                    }
	                    acc = std::sqrt(acc + 1e-10);
	                    for (size_t j=0; j<Config.FilterSize; ++j) {
	                        Filter(i, j) = Filter(i, j)/acc;
	                    }
	                }
	            }
	        }
	    }
	    if (Config.Learn) {
	    	runret.MeanLoss = runret.MeanLoss/iter;
	    }
	    return runret;
    }

private:
	void RestoreImpl(const TFilterMatch* matches, size_t matches_len, std::vector<double>& restored, size_t from=0) const {
	    for (size_t mi=0; mi < matches_len; ++mi) {
	    	const TFilterMatch& m = matches[mi];
	        for(size_t i=0; i<Config.FilterSize; ++i) {
	            restored[m.T + i - from] += m.S * Filter(m.Fi, i) + Config.NoiseSd * NormDistr(Generator);
	        }
	    }

	    double denom = 0.0;
	    for(const auto& v: restored) { 
	    	denom += v*v;
	    }
	    denom = sqrt(denom + 1e-10);
	    for(auto& v: restored) {
	    	v /= denom;
	    }
	}

};


#include <stdio.h>

#ifdef DLL
	#ifdef EXPORTS
		#define API __declspec(dllexport)
	#else
		#define API __declspec(dllimport)
	#endif
#else		
	#define API extern 
#endif /* DLL */

extern "C" {
	#include <string.h>

	API void run_mpl(
		const double* data, unsigned int len,
		double* filters,
		unsigned int filters_num, unsigned int filter_size,
		double threshold, 
		bool learn,
		double learning_rate,
		unsigned int learn_iterations,
		double momentum,
		unsigned int batch_size, unsigned int jobs, 
		struct TFilterMatch** matches, unsigned int* matches_size,
		double** residual, unsigned int* residual_size,
		double* mean_loss) 
	{
		TMatchingPursuitConfig config;
		config.Learn = learn;
		config.LearningRate = learning_rate;
		config.LearnIterations = learn_iterations;
		config.FiltersNum = filters_num;
		config.FilterSize = filter_size;
		config.BatchSize = batch_size;
		config.Jobs = jobs;
		config.Threshold = threshold;
		config.Momentum = momentum;

		TMatchingPursuit mp(config, filters);
		std::vector<double> data_vec(len);
		memcpy(&data_vec[0], data, sizeof(double) * len);
		
		TMatchingPursuit::TReturn ret = mp.Run(data_vec);
		
		*matches_size = ret.Matches.size();
		*matches = (TFilterMatch*)malloc(sizeof(TFilterMatch) * ret.Matches.size());
		memcpy(*matches, &ret.Matches[0], sizeof(TFilterMatch) * ret.Matches.size());
		
		*residual_size = ret.Residual.size();
		*residual = (double*)malloc(sizeof(double) * ret.Residual.size());
		memcpy(*residual, &ret.Residual[0], sizeof(double) * ret.Residual.size());

		*mean_loss = ret.MeanLoss;
	}

	API void free_matches(struct TFilterMatch** data) {
		free(*data);
	} 

	API void free_double(double** data) {
		free(*data);
	}

	API void restore(
		const struct TFilterMatch* matches, 
		unsigned int matches_size, 
		double* filters,
		unsigned int filters_num, unsigned int filter_size,
		double** restored_arr, unsigned int* restored_size) 
	{
		TMatchingPursuitConfig config;
		config.FiltersNum = filters_num;
		config.FilterSize = filter_size;

		TMatchingPursuit mp(config, filters);
		auto restored = mp.Restore(matches, matches_size);

		*restored_size = restored.size();
		*restored_arr = (double*)malloc(sizeof(double) * restored.size());
		memcpy(*restored_arr, &restored[0], sizeof(double) * restored.size());
	}
}