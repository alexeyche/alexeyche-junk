#ifndef RMPL_H
#define RMPL_H


#include <dnn/mpl/mpl.h>

#undef PI
#define STRICT_R_HEADERS
#include <Rcpp.h>

#include "RSim.h"


class RMatchingPursuit : public dnn::MatchingPursuit {
public:
	RMatchingPursuit(const Rcpp::List conf) 
	: MatchingPursuit(
		*RProto::convertBack<dnn::MatchingPursuitConfig>(conf, "MatchingPursuitConfig")
	  ) {

	}
	Rcpp::List run(const Rcpp::NumericVector ts_m) {
		Factory::inst().registrationOff();
		TimeSeries *ts = RProto::convertBack<dnn::TimeSeries>(
			Rcpp::List::create(
				Rcpp::Named("values") = ts_m
			),
			"TimeSeries"
		);
		Factory::inst().registrationOn();
		dnn::MatchingPursuit::MPLReturn ret = MatchingPursuit::run(*ts, 0);
		delete ts;

		Rcpp::List matches_l;

		for(auto &m: ret.matches) {
			matches_l.push_back(
				Rcpp::List::create(
					Rcpp::Named("t") = m.t,
					Rcpp::Named("fi") = m.fi,
					Rcpp::Named("s") = m.s
				)
			);
		}
		return Rcpp::List::create(
			Rcpp::Named("spikes") = matches_l, 
			Rcpp::Named("restored") = Rcpp::wrap(ret.restored),
			Rcpp::Named("accum_error") = Rcpp::wrap(ret.accum_error)
		);
	}
	void setFilter(const Rcpp::NumericMatrix m) {
		dnn::MatchingPursuit::setFilter(
			*RProto::convertBack<DoubleMatrix>(
				Rcpp::List::create(
					Rcpp::Named("DoubleMatrix") = m
				), 
				"DoubleMatrix"
			)
		);
	}
	void setConf(const Rcpp::List conf) {
		Factory::inst().registrationOff();
		MatchingPursuitConfig *in_c = RProto::convertBack<dnn::MatchingPursuitConfig>(conf, "MatchingPursuitConfig");
		MatchingPursuit::c = *in_c;
		delete in_c;
		Factory::inst().registrationOn();
	}

	Rcpp::NumericMatrix getFilter() {
		return RProto::convertToList(&filter)[0];
	}

	Rcpp::NumericVector restore(const Rcpp::List matches_l) {
		vector<FilterMatch> matches;
		for(auto &mr: matches_l) {
			Rcpp::List m = mr;
			matches.push_back(
				FilterMatch(Rcpp::as<size_t>(m["fi"]), Rcpp::as<double>(m["s"]), Rcpp::as<double>(m["t"]))
			);
		}
		return Rcpp::wrap(dnn::MatchingPursuit::restore(matches));
	}
	void print() {
        cout << "MatchingPursuit instance.\n";
        cout << "config:\n";
        Stream(cout, Stream::Text).writeObject(&c);
    }
};


#endif