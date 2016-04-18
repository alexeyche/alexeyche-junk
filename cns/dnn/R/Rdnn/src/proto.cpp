#include "proto.h"

#include <dnn/util/ts/time_series.h>
#include <dnn/util/ts/spikes_list.h>
#include <dnn/util/stat_gatherer.h>
#include <dnn/util/serial/bin_serial.h>

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
Rcpp::List TProto::Translate<TSpikesList>(const TSpikesList& ent) {
    TVector<TVector<double>> sp;
    for(auto &seq : ent.Data) {
        sp.push_back(seq.Values);
    }

    Rcpp::List ret = Rcpp::List::create(
          Rcpp::Named("values") = Rcpp::wrap(sp)
        , Rcpp::Named("info") = Translate<TTimeSeriesInfo>(ent.Info)
    );
    ret.attr("class") = "SpikesList";
    return ret;
}


template <>
Rcpp::List TProto::Translate<TStatistics>(const TStatistics& ent) {
    Rcpp::List ret = Rcpp::List::create(
          Rcpp::Named("values") = Rcpp::wrap(ent.Values)
        , Rcpp::Named("name") = ent.Name
        , Rcpp::Named("from") = ent.From
        , Rcpp::Named("to") = ent.To
    );
    ret.attr("class") = "Statistics";
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
    if(l.containsElementNamed("info")) {
        ui32 dimsize = ts.Info.DimSize;
        ts.Info = TranslateBack<TTimeSeriesInfo>(l["info"]);
        ts.Info.DimSize = dimsize;
    }
    return ts;
}

template <>
TSpikesList TProto::TranslateBack<TSpikesList>(const Rcpp::List& l) {
    TSpikesList sl;
    Rcpp::List spikes = l["values"];
    for(auto &sp_v: spikes) {
        TSpikesListData sp_seq;
        sp_seq.Values = Rcpp::as<TVector<double>>(sp_v);
        sl.Data.push_back(sp_seq);
    }

    sl.Info.DimSize = sl.Data.size();

    if(l.containsElementNamed("info")) {
        sl.Info = TranslateBack<TTimeSeriesInfo>(l["info"]);
    }
    return sl;
}

Rcpp::List TProto::TranslateModel(const NDnnProto::TConfig& config) {
    Rcpp::List res;
    
    for (const auto& layer: config.layer()) {
        ui32 layerSynapseCounter = 0;
        for (const auto& neuronInner: layer.spikeneuronimplinnerstate()) {
            Rcpp::List neuronInfo;
            Rcpp::NumericVector weights;
            Rcpp::IntegerVector ids_pre;
            
            for (ui32 synId=0; synId < neuronInner.synapsessize(); ++synId, ++layerSynapseCounter) {
                auto synInner = layer.synapseinnerstate(layerSynapseCounter);
                weights.push_back(synInner.weight());
                ids_pre.push_back(synInner.idpre());
            }
            res.push_back(
                Rcpp::List::create(
                    Rcpp::Named("weights") = weights,
                    Rcpp::Named("ids_pre") = ids_pre
                )
            );
        }    
    }
    
    return res;
}


Rcpp::List TProto::ReadFromFile(TString protofile) {
	std::ifstream input(protofile, std::ios::binary);
    TBinSerial serial(input);

	Rcpp::List l;
    switch (serial.ReadProtobufType()) {
        case EProto::TIME_SERIES:
            l = Translate(serial.ReadObject<TTimeSeries>());
            break;
        case EProto::SPIKES_LIST:
            l = Translate(serial.ReadObject<TSpikesList>());
            break;
        case EProto::STATISTICS:
            {
                TStatistics stat;
                while (serial.ReadObject<TStatistics>(stat)) {
                    Rcpp::List subList;
                    subList = Translate(stat);
                    l[stat.Name] = subList;
                }
            }
            break;
        case EProto::CONFIG:
            {
                NDnnProto::TConfig config;
                if (!serial.ReadProtobufMessage(config)) {
                    ERR("Failed to read config protobuf: " << protofile);
                }
                l = TranslateModel(config);
            }
            break;
        default:
            ERR("Unknown protobuf type " << protofile);
    }
	return l;
}

void TProto::WriteToFile(Rcpp::List l, TString protofile) {
	std::ofstream output(protofile, std::ios::binary);
    TBinSerial serial(output);

    TString name = l.attr("class");
    if (name == "TimeSeries") {
        serial.WriteObject(TranslateBack<TTimeSeries>(l));
    	return;
    }
    if (name == "SpikesList") {
        serial.WriteObject(TranslateBack<TSpikesList>(l));
        return;
    }
    ERR("Failed to find appropriate entity for data in R structure with class " << name);
}