#pragma once

#include <dnn/util/pretty_print.h>
#include <dnn/util/ts/spikes_list.h>
#include <dnn/util/act_vector.h>
#include <dnn/neuron/spike_neuron_impl.h>

namespace NDnn {

    class TConn {
    public:
        TConn(TAsyncSpikeQueue& dstQueue, ui32 synId, double dendriteDelay)
            : DstQueue(dstQueue)
            , SynId(synId)
            , DendriteDelay(dendriteDelay)
        {}

        TAsyncSpikeQueue& DstQueue;
        ui32 SynId;

        double DendriteDelay;

        friend std::ostream& operator<<(std::ostream& str, const TConn& self) {
            str << "(" << self.DstQueue.Info << ", " << self.SynId << ", Delay: " << self.DendriteDelay << ")";
            return str;
        }
    };

    class TNetwork {
    public:
        TNetwork() {}

        TNetwork(const TNetwork& other) {
            (*this) = other;
        }

        TNetwork& operator=(const TNetwork& other) {
            if (this != &other) {
                SpikesList = other.SpikesList;
            }
            return *this;
        }

        void Init(ui32 popSize) {
            ConnMap.clear();
            ConnMap.resize(popSize);
            SpikesList.Data.clear();
            SpikesList.Data.resize(popSize);
        }

        template <typename TLayer>
        void AddLayer(TLayer& layer) {
            for(auto& n: layer) {
                const auto& syns = n.GetSynapses();
                for (ui32 con_i = 0; con_i < syns.size(); ++con_i) {
                    const auto& syn = syns.Get(con_i);
                    // const auto& syn = syns[con_i];
                    ConnMap[ syn.IdPre() ].emplace_back(n.GetMutAsyncSpikeQueue(), con_i, syn.DendriteDelay());
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
                conn.DstQueue.EnqueueSpike(
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

        TSpikesList& GetMutSpikesList() {
            return SpikesList;
        }

        const TSpikesList& GetSpikesList() const {
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