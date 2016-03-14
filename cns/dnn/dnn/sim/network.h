#pragma once

#include <dnn/util/pretty_print.h>
#include <dnn/util/ts/spikes_list.h>
#include <dnn/util/act_vector.h>

namespace NDnn {

    class TConn {
    public:
        TConn(ui32 synId, double dendriteDelay)
            : SynId(synId)
            , DendriteDelay(dendriteDelay)
        {}

        double DendriteDelay;
        ui32 SynId;

        friend std::ostream& operator<<(std::ostream& str, const TConn& self) {
            str << "(" << self.SynId << ", Delay: " << self.DendriteDelay << ")";
            return str;
        }
    };

    class TNetwork {
    public:
        TNetwork() {}

        void Init(ui32 popSize) {
            ConnMap.resize(popSize);
            SpikesList.Data.resize(popSize);
        }

        template <typename TLayer>
        void AddLayer(TLayer& layer) {
            for(auto& n: layer) {
                const auto& syns = n.GetSynapses();
                for (ui32 con_i = 0; con_i < syns.size(); ++con_i) {
                    const auto& syn = syns.Get(con_i);

                    ConnMap[ syn.IdPre() ].emplace_back(con_i, syn.DendriteDelay());
                }
            }
        }

        template <typename TNeuron>
        void PropagateSpike(const TNeuron& neuron, double t) {
            SpikesList[neuron.GetGlobalId()].push_back(t);

            ENSURE((t < 1000.0) || (1000.0*((SpikesList[neuron.GetGlobalId()].size())/t) < 300.0),
                "Rate limit exceeded: " << SpikesList[neuron.GetGlobalId()].size() << " spikes of neuron " << neuron.GetGlobalId() << " at " << t
            );

            for(auto& conn : ConnMap[neuron.GetGlobalId()]) {
                conn.neuron.enqueueSpike(
                    TSynSpike(
                          neuron.GetGlobalId() /* source of spike */
                        , conn.SynId /* destination synapse */
                        , t  + neuron.GetAxonDelay() + conn.DendriteDelay /* time of spike */
                    )
                );
            }
        }

        friend std::ostream& operator<<(std::ostream& str, const TNetwork& self) {
            str << "Network: \n";
            str << "\tConnMap: \n";
            for(ui32 i=0; i<self.ConnMap.size(); ++i) {
                str << "neuron " << i << " cause spike in: ";
                str << self.ConnMap[i];
                str << "\n";
            }
            return str;
        }

        TSpikesList& GetSpikesList() {
            return SpikesList;
        }

        TMaybe<ui32> GetClassId(const TTime& t) {
            return SpikesList.Info.GetClassId(t.T);
        }

    private:
        TVector<TVector<TConn>> ConnMap;
        TSpikesList SpikesList;
    };





}