#ifndef LZ78_H
#define LZ78_H


#include <unordered_map>

typedef std::unordered_map<std::string, size_t> TDict;

typedef std::vector<bool> TBitSpikes;

const char* boolToChar(bool b) {
   if(b) {
       return "1";
   } else {
       return "0";  
   }
}
TDict lz78Phrases(TBitSpikes bits) {
    TDict d;
    std::string bit_arr("");
    for(size_t bi = 0; bi<bits.size(); bi++) {
        bit_arr = bit_arr + std::string(boolToChar(bits.at(bi)));
        if(d.find(bit_arr) == d.end()) {
            d[bit_arr] = 1;
            bit_arr = "";
        } else {
            d[bit_arr] += 1;
        }
    }
    return d;
}

typedef std::vector<double> TSpikeTimes;
TBitSpikes spikeTimesToBits(TSpikeTimes st, double T0, double Tmax, double dt) {
    TBitSpikes bs;
    size_t st_i = 0;
    vec t = linspace<vec>(T0, Tmax, (Tmax-T0)/dt);
    for(size_t ti=0; ti<t.n_elem-1; ti++) {
        if( (st[st_i]>=t(ti) ) && ( st[st_i]<t(ti+1)) ) {
            bs.push_back(1);       
            st_i++;
        }
        bs.push_back(0);       
    }
    return bs;
}

#endif 
