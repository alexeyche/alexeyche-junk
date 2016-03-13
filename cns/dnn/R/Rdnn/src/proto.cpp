#include "proto.h"

using namespace NDnn;

template <>
Rcpp::List TProto::Translate<TTimeSeriesInfo>(const TTimeSeriesInfo& ent) {
	Rcpp::List ret;
    for(const auto& lab_start_info: ent.LabelsStart) {
        const auto& lab_spec = ent.UniqueLabels[lab_start_info.LabelId];
        ret.push_back(
            Rcpp::List::create(
                Rcpp::Named("label") = lab_spec.Name
              , Rcpp::Named("start_time") = lab_start_info.Start
              , Rcpp::Named("duration") = lab_spec.Duration
            )
        );
    }
    ret.attr("class") = "TimeSeriesInfo";
    return ret;
}


template <>
Rcpp::List TProto::Translate<TTimeSeries>(const TTimeSeries& ent) {
	Rcpp::NumericMatrix ts_vals(ent.Dim(), ent.Length());
    for(size_t i=0; i<ent.Data.size(); ++i) {
        for(size_t j=0; j<ent.Data[i].Values.size(); ++j) {
            ts_vals(i, j) = ent.Data[i].Values[j];
        }
    }
    Rcpp::List ret = Rcpp::List::create(
          Rcpp::Named("values") = ts_vals
        , Rcpp::Named("info") = Translate<TTimeSeriesInfo>(ent.Info)
    );
    ret.attr("class") = "TimeSeries";
    return ret;
}

template <>
TTimeSeriesInfo TProto::TranslateBack<TTimeSeriesInfo>(const Rcpp::List& l) {
	TTimeSeriesInfo ret;
    for(size_t li=0; li<l.size(); ++li) {
        Rcpp::List elem(l[li]);
        ret.AddLabelAtPos(elem["label"], elem["start_time"], elem["duration"]);
    }
    return ret;
}

template <>
TTimeSeries TProto::TranslateBack<TTimeSeries>(const Rcpp::List& l) {
    TTimeSeries ts;
    SEXP values = l["values"];
    if(Rf_isMatrix(values)) {
        Rcpp::NumericMatrix m(values);
        ts.Info.DimSize = m.nrow();
        ts.Data.resize(ts.Info.DimSize);
        for(size_t i=0; i<m.nrow(); ++i) {
            for(size_t j=0; j<m.ncol(); ++j) {
                ts.Data[i].Values.push_back(m(i,j));
            }
        }
    } else {
        ts.Info.DimSize = 1;
        ts.Data.resize(ts.Info.DimSize);
        ts.Data[0].Values = Rcpp::as<std::vector<double>>(values);
    }
    Rcpp::List info;
    if(l.containsElementNamed("info")) {
        ui32 dimsize = ts.Info.DimSize;
        ts.Info = TranslateBack<TTimeSeriesInfo>(info);
        ts.Info.DimSize = dimsize;
    }
    return ts;
}

Rcpp::List TProto::ReadFromFile(TString protofile) {
	std::fstream input(protofile, std::ios::in | std::ios::binary);
	
	Rcpp::List l;
	if (ReadEntity<TTimeSeries>(input, l)) {
		return l;
	}
	ERR("Failed to find appropriate entity for data in file " << protofile);
	return l; // For warning
}

void TProto::WriteToFile(Rcpp::List l, TString protofile) {
	std::fstream f(protofile, std::ios::out | std::ios::binary);
    TString name = l.attr("class");
    if (name == "TimeSeries") {
    	WriteEntity(TranslateBack<TTimeSeries>(l), f);
    	return;
    }
	    
    ERR("Failed to find appropriate entity for data in R structure with class " << name);
}