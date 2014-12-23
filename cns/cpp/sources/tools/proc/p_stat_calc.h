#pragma once

#include <snnlib/neurons/neuron_stat.h>
#include <snnlib/util/util.h>

#include <thread>


struct PatternsPair {
    PatternsPair(size_t _i, size_t _j, IndexSlice _left, IndexSlice _right) : i(_i), j(_j), left(_left), right(_right) {}
    size_t i;
    size_t j;
    IndexSlice left;
    IndexSlice right;
};

double calculatePStatDistance(const vector<NeuronStat*> &st, const IndexSlice &left, const IndexSlice &right) {
    double d = 0.0;
    for(size_t ni=0; ni < st.size(); ni++) {
        const vector<double> &p_stat = st[ni]->p;
        double d_integral = 0.0;
        //cout << "calc dist " << left.from << ":" << left.to << " - " << right.from << ":" << right.to << "\n";
        size_t i = left.from, j = right.from;
        for( ; (i<left.to)||(j<right.to); i++, j++) {
            double p_left = 0.0, p_right = 0.0;
            if(i<p_stat.size()) p_left = p_stat[i];
            if(j<p_stat.size()) p_right = p_stat[j];

            d_integral += (p_left - p_right)*(p_left - p_right);
        }
        d += d_integral;
    }
    return d/st.size();
}

void calculateDistanceBetweenPatterns(const vector<NeuronStat*> &st, const vector<PatternsPair> &ppair, DoubleMatrix &dist, size_t from, size_t to) {
    for(size_t i=from; i<to; i++) {
        const PatternsPair &p = ppair[i];
        double d = calculatePStatDistance(st, p.left, p.right);
        dist.setElement(p.i, p.j, d);
        dist.setElement(p.j, p.i, d);
    }
}


DoubleMatrix calcPStatDistance(const vector<NeuronStat*> &st, const vector<IndexSlice> &patterns, int jobs) {
    vector<PatternsPair> ppair;
    for(size_t i=0; i < patterns.size(); i++) {
        for(size_t j=0; j < i; j++) {
            ppair.push_back( PatternsPair(i, j, patterns[i], patterns[j]) );
        }
    }
    DoubleMatrix dist(patterns.size(), patterns.size(), 0.0);

    vector<IndexSlice> slices = dispatchOnThreads(ppair.size(), jobs);

    vector<std::thread> threads;
    for(auto it=slices.begin(); it != slices.end(); ++it) {
        IndexSlice &sl = *it;
        threads.push_back( std::thread(calculateDistanceBetweenPatterns, std::cref(st), std::cref(ppair), std::ref(dist), sl.from, sl.to) );
    }
    for(auto it=threads.begin(); it != threads.end(); ++it) {
        it->join();
    }

    return dist;
}
