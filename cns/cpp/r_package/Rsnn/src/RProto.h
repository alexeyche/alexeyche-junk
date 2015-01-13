#ifndef RPROTO_READ_H
#define RPROTO_READ_H

#define STRICT_R_HEADERS
#include <Rcpp.h>

#include "common.h"

#include <snnlib/serialize/proto_rw.h>
#include <snnlib/serialize/serialize.h>
#include <snnlib/neurons/adex_neuron.h>
#include <snnlib/learning/max_likelihood.h>
#include <snnlib/learning/optimal_stdp.h>
#include <snnlib/learning/bcm_rule.h>
#include <snnlib/learning/triple_stdp.h>
#include <snnlib/learning/stdp.h>

class RProto {
public:
    RProto(std::string _protofile) : protofile(_protofile) {
    }
    Rcpp::List read() {
        if(values.size() == 0) {
            if(getFileSize(protofile) == 0) return values;

            ProtoRw *rw = new ProtoRw(protofile, ProtoRw::Read);

            SerializableBase *o = rw->read();
            if(!o) {
                ERR("Can't read protofile " << protofile << "\n");
            }

            if(o->getName() == "Constants") {
                delete rw;
                Constants *c = static_cast<Constants*>(o);
                values = readModelFromFile(c);
                return values;
            }
            
            SerialPack v({o});
            
            while(true) {
                SerializableBase *o = rw->read();
                if(!o) break;
                v.push_back(o);
            }
            if ((v.size() == 1) && (v[0]->getName() == "LabeledSpikesList")) {
                values = convertToList(v[0]);
            } else {
                for(size_t vi=0; vi<v.size(); vi++) {
                    stringstream ss;
                    ss << v[vi]->getName() << "_" << vi;
                    values[ss.str()] = convertToList(v[vi]);
                }
            }
            delete rw;
        }            
        return values;
    }
    void print() {
        cout << "RProto instance. run instance$read() method to read protobuf\n";
    }
    static ProtoPack listToProtobuf(const Rcpp::List &list, const string &message_name) {
        if(message_name == "LabeledTimeSeriesList") {
            Protos::LabeledTimeSeriesList *lts_mess = new Protos::LabeledTimeSeriesList;
            const Rcpp::List &lts_list = list;
            for(auto it=lts_list.begin(); it != lts_list.end(); ++it) {
                Protos::LabeledTimeSeries *lts = lts_mess->add_list();
                Rcpp::List lts_r = *it;
                const string &label = lts_r["label"];
                lts->set_label(label);

                Protos::TimeSeries *ts = new Protos::TimeSeries();
                vector<double> ts_r = lts_r["ts"];
                for(auto ts_it=ts_r.begin(); ts_it != ts_r.end(); ++ts_it) {
                    ts->add_data(*ts_it);
                }
                lts->set_allocated_ts(ts);
            }
            return ProtoPack({lts_mess});
        } else    
        if(message_name == "LabeledSpikesList") {
            Protos::LabeledSpikesList *lsl_mess = new Protos::LabeledSpikesList;

            Protos::SpikesList *sl = new Protos::SpikesList;
            Rcpp::List sp_list = list["spikes_list"];
            for(size_t ni=0; ni<sp_list.size(); ni++) {
                Rcpp::NumericVector spikes = sp_list[ni];
                Protos::SpikesSequence *s = sl->add_spikes_list();
                for(auto it=spikes.begin(); it != spikes.end(); ++it) {
                    s->add_seq(*it);
                }
            }
            lsl_mess->set_allocated_sl(sl);

            Protos::PatternsTimeline *ptl = new Protos::PatternsTimeline;
            Rcpp::CharacterVector labels_r = list["labels"];
            vector<string> labels_tl = Rcpp::as< vector<string> >(labels_r);
            Rcpp::NumericVector timeline = list["timeline"];
            if(labels_tl.size() != timeline.size()) {
                ERR("Can't write spikes list with labels and timeline different size\n");
            }
            vector<string> labels;
            for(size_t i=0; i < labels_tl.size(); i++) {
                string lab = labels_tl[i];
                double tl = timeline[i];
                int pos = -1;
                if(labels.size()>0) {
                    auto pos_it = find(labels.begin(), labels.end(), lab);
                    if(pos_it != labels.end()) {
                        pos = pos_it - labels.begin();
                    }
                }
                if(pos<0) {
                    ptl->add_labels(lab);
                    labels.push_back(lab);
                    pos = labels.size()-1;
                }
                ptl->add_labels_id_timeline(pos);
                ptl->add_timeline(tl);
            }
            double Tmax = timeline[timeline.size()-1];
            ptl->set_tmax(Tmax);
            Rcpp::NumericVector dt = list["dt"];
            if(dt.size() != 1) {
                ERR("dt must be numeric vector with size 1")
            }
            ptl->set_dt(dt[0]);
            Rcpp::NumericVector gap_between_patterns = list["gap_between_patterns"];
            if(gap_between_patterns.size() != 1) {
                ERR("gap_between_patterns must be numeric vector with size 1")
            }

            ptl->set_gap_between_patterns(gap_between_patterns[0]);

            lsl_mess->set_allocated_ptl(ptl);
            return ProtoPack({lsl_mess});
        } else {
            ERR("Can't recognize serializable name: " << message_name << "\n");
        }
    }
    void write(const Rcpp::List &list, const string &message_name) {
        ProtoRw rw(protofile, ProtoRw::Write);
        ProtoPack p = listToProtobuf(list, message_name);
        rw.writeProtoPack(p, message_name);
    }
    static Rcpp::List readModel(Sim *s) {
        size_t total_size = s->input_neurons_count + s->net_neurons_count;
        Rcpp::NumericMatrix w(total_size, total_size);
        for(auto it=s->layers.begin(); it != s->layers.end(); ++it) {
            Layer *l = *it;
            for(auto it_n=l->neurons.begin(); it_n != l->neurons.end(); ++it_n) {
                Neuron *n = *it_n;
                for(auto it_s=n->syns.begin(); it_s != n->syns.end(); ++it_s) {
                    Synapse *syn = *it_s;
                    w(syn->id_pre, n->id) = syn->w;
                }
            }
        }        
        Rcpp::List out;
        out["w"] = w;
        out["sim_time"] = s->rg.getSimTime();
        return out;
    }
    Rcpp::List readModelFromFile(Constants *c) { 
        Sim s(*c);
        s.loadModel(protofile);
        Rcpp::List l = readModel(&s);
        Factory::inst().cleanAll();
        return l;
    }

    static Rcpp::List convertToList(SerializableBase *s) {
        Rcpp::List out;
        if(s->getName() == "LabeledSpikesList") {
            LabeledSpikesList *lsl = dynamic_cast<LabeledSpikesList*>(s);
            if(!lsl) { ERR("Can't cast"); }
            Rcpp::List spikes;
            for(size_t ni=0; ni<lsl->sl.N; ni++) {
                stringstream ss;
                ss << ni;
                spikes[ss.str()] = Rcpp::NumericVector(Rcpp::wrap(lsl->sl.sp_list[ni]));
            }
            out["spikes"] = spikes;
            out["labels"] = Rcpp::wrap(lsl->ptl.labels);
            out["labels_id_timeline"] = Rcpp::wrap(lsl->ptl.labels_id_timeline);
            out["end_of_patterns"] = Rcpp::wrap(lsl->ptl.timeline);
            out["dt"] = lsl->ptl.dt;
            out["Tmax"] = lsl->ptl.Tmax;
            out["gapBetweenPatterns"] = lsl->ptl.gapBetweenPatterns;
        }
        if(s->getName() == "SpikesList") {
            SpikesList *sl = dynamic_cast<SpikesList*>(s);
            if(!sl) { ERR("Can't cast"); }
            for(size_t ni=0; ni<sl->N; ni++) {
                stringstream ss;
                ss << ni;
                out[ss.str()] = Rcpp::NumericVector(Rcpp::wrap(sl->sp_list[ni]));
            }
        } 
        if((s->getName() == "NeuronStat")||(s->getName() == "AdExNeuronStat")) {
            NeuronStat *st = dynamic_cast<NeuronStat*>(s);
            if(!st) { ERR("Can't cast"); }
            out["p"] = Rcpp::wrap(st->p);
            if(st->M.size()>0) out["M"] = Rcpp::wrap(st->M);
            if(st->u.size()>0) out["u"] = Rcpp::wrap(st->u);
            if(st->syns.size()>0) out["syns"] = Rcpp::wrap(st->syns);
            if(st->w.size()>0) out["w"] = Rcpp::wrap(st->w);
            if(st->ga.size()>0) out["ga"] = Rcpp::wrap(st->ga);
        } 
        if(s->getName() == "AdExNeuronStat") {
            AdExNeuronStat *st = dynamic_cast<AdExNeuronStat*>(s);
            if(!st) { ERR("Can't cast"); }
            out["a"] = Rcpp::wrap(st->a);
        }
        if(s->getName() == "RewardStat") {
            RewardStat *st = dynamic_cast<RewardStat*>(s);
            if(!st) { ERR("Can't cast"); }
            out["r"] = Rcpp::wrap(st->r);
            out["mean_r"] = Rcpp::wrap(st->mean_r);
        } 
        if(s->getName() == "MaxLikelihoodStat") {
            MaxLikelihoodStat *st = dynamic_cast<MaxLikelihoodStat*>(s);
            if(!st) { ERR("Can't cast"); }
            out["traces"] = Rcpp::wrap(st->eligibility_trace);
        } 
        if(s->getName() == "StdpStat") {
            StdpStat *st = dynamic_cast<StdpStat*>(s);
            if(!st) { ERR("Can't cast"); }
            out["y_trace"] = Rcpp::wrap(st->y_trace);
            out["x_trace"] = Rcpp::wrap(st->x_trace);
        }
        if(s->getName() == "TripleStdpStat") {
            TripleStdpStat *st = dynamic_cast<TripleStdpStat*>(s);
            if(!st) { ERR("Can't cast"); }
            out["r1"] = Rcpp::wrap(st->r1);
            out["r2"] = Rcpp::wrap(st->r2);
            out["o1"] = Rcpp::wrap(st->o1);
            out["o2"] = Rcpp::wrap(st->o2);
        }
        if(s->getName() == "OptimalStdpStat") {
            OptimalStdpStat *st = dynamic_cast<OptimalStdpStat*>(s);
            if(!st) { ERR("Can't cast"); }
            out["p_acc"] = Rcpp::wrap(st->p_acc);
            out["B"] = Rcpp::wrap(st->B);
            out["C"] = Rcpp::wrap(st->C);
        } 
        if(s->getName() == "BCMRuleStat") {
            BCMRuleStat *st = dynamic_cast<BCMRuleStat*>(s);
            if(!st) { ERR("Can't cast"); }
            out["p_acc"] = Rcpp::wrap(st->p_acc);
            out["y"] = Rcpp::wrap(st->y);
            out["x"] = Rcpp::wrap(st->x);
        } 
        if(out.size()==0) {
            ERR("Unknown serializable name: " << s->getName() << "\n");
        }
        return out;
    }
    
    Rcpp::List values;
    std::string protofile;
};



#endif
