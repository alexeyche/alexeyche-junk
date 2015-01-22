#pragma once

#include <snnlib/neurons/neuron_stat.h>
#include <snnlib/util/util.h>

#include <thread>


struct PatternsPair {
    PatternsPair(const vector<NeuronStat*> &_src, const vector<NeuronStat*> &_dst, IndexSlice _left, IndexSlice _right, size_t _i, size_t _j) : src(_src), dst(_dst), left(_left), right(_right), i(_i), j(_j) {}
    const vector<NeuronStat*> &src;
    const vector<NeuronStat*> &dst;

    IndexSlice left;
    IndexSlice right;
    size_t i;
    size_t j;
};

double calculatePStatDistance(const vector<NeuronStat*> &src, const vector<NeuronStat*> &dst, const IndexSlice &left, const IndexSlice &right) {
    double d = 0.0;
    for(size_t ni=0; ni < src.size(); ni++) {
        const vector<double> &p_stat_src = src[ni]->p;
        const vector<double> &p_stat_dst = dst[ni]->p;
        double d_integral = 0.0;
        //cout << "calc dist " << left.from << ":" << left.to << " - " << right.from << ":" << right.to << "\n";
        size_t i = left.from, j = right.from;
        for( ; (i<left.to)||(j<right.to); i++, j++) {
            double p_left = 0.0, p_right = 0.0;
            if(i<p_stat_src.size()) p_left = p_stat_src[i];
            if(j<p_stat_dst.size()) p_right = p_stat_dst[j];

            d_integral += (p_left - p_right)*(p_left - p_right);
        }
        d += d_integral;
    }
    return d/src.size();
}

void calculateDistanceBetweenPatterns(const vector<PatternsPair> &ppair, DoubleMatrix &dist, size_t from, size_t to) {
    for(size_t i=from; i<to; i++) {
        const PatternsPair &p = ppair[i];
        double d = calculatePStatDistance(p.src, p.dst, p.left, p.right);
        dist.setElement(p.i, p.j, d);
        dist.setElement(p.j, p.i, d);
    }
}


DoubleMatrix calcPStatDistance(const vector<NeuronStat*> &st, const vector<NeuronStat*> &test_st, const vector<IndexSlice> &patterns, const vector<IndexSlice> &test_patterns, int jobs) {
    if(test_st.size()>0) {
        assert(st.size() == test_st.size());
    }

    vector<PatternsPair> ppair;
    for(size_t i=0; i < (patterns.size()+test_patterns.size()); i++) {
        for(size_t j=0; j < i; j++) {
            const vector<NeuronStat*> *src = &st;
            const vector<NeuronStat*> *dst = &st;
            const IndexSlice *src_slice = &patterns[i];
            const IndexSlice *dst_slice = &patterns[j];
            if(i>=patterns.size()) {
                src = &test_st;
                src_slice = &test_patterns[i-patterns.size()];
            }
            if(j>=patterns.size()) {
                dst = &test_st;
                dst_slice = &test_patterns[j-patterns.size()];
            }
            //cout << i << ":" << j << " == " << src_slice->from << ":" << src_slice->to << " " << dst_slice->from << ":" << dst_slice->to  << "\n";
            ppair.push_back( PatternsPair(*src, *dst, *src_slice, *dst_slice, i, j) );
        }
    }
    DoubleMatrix dist(patterns.size()+test_patterns.size(), patterns.size()+test_patterns.size(), 0.0);

    vector<IndexSlice> slices = dispatchOnThreads(ppair.size(), jobs);

    vector<std::thread> threads;
    for(auto it=slices.begin(); it != slices.end(); ++it) {
        IndexSlice &sl = *it;
        threads.push_back( std::thread(calculateDistanceBetweenPatterns, std::cref(ppair), std::ref(dist), sl.from, sl.to) );
    }
    for(auto it=threads.begin(); it != threads.end(); ++it) {
        it->join();
    }

    return dist;
}
