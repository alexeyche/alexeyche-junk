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
TDict lz78Phrases(TBitSpikes &bits) {
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

int lz78Complexity(TBitSpikes bits) {
    TDict d = lz78Phrases(bits);
    return d.size()*log(d.size())/bits.size();
}


typedef std::vector<double> TSpikeTimes;
TBitSpikes spikeTimesToBits(TSpikeTimes &st, double T0, double Tmax, double dt) {
    TBitSpikes bs;
    size_t st_i = 0;
    vec t = linspace<vec>(T0, Tmax, (Tmax-T0)/dt);
    for(size_t ti=0; ti<t.n_elem-1; ti++) {
        if( (st[st_i]>=t(ti) ) && ( st[st_i]<t(ti+1)) ) {
            bs.push_back(1);       
            st_i++;
            continue;
        }
        bs.push_back(0);       
    }
    return bs;
}

int phrasesDiff(const TDict &p1, const TDict &p2) {
    int diff=p1.size();
    TDict::const_iterator it = p1.begin();
    while(it != p1.end()) {
        if(p2.find(it->first) != p2.end()) {
            diff--;
        } 
        it++;
    }
    return diff;
}

void printBitSpikes(TBitSpikes &bs) {
    for(size_t bi=0; bi<bs.size(); bi++) {
        Log::Info << bs[bi];
    }
    Log::Info << "\n";
}

double lz78Distance(TSpikeTimes &st1, TSpikeTimes &st2, double T0, double Tmax, double dt) {
    TBitSpikes bs1 = spikeTimesToBits(st1,T0,Tmax,dt);
    TBitSpikes bs2 = spikeTimesToBits(st2,T0,Tmax,dt);
//    printBitSpikes(bs1);
//    printBitSpikes(bs2);
    const int &n = bs1.size(); // bs2.size() == bs1.size()
    TDict p1 = lz78Phrases(bs1);
    TDict p2 = lz78Phrases(bs2);
    double kx = p1.size()*log(p1.size())/n;
    double ky = p2.size()*log(p2.size())/n;
    int cxy = phrasesDiff(p1, p2);
    int cyx = phrasesDiff(p2, p1);
    double kxy, kyx; 
    if(cxy == 0) { 
        kxy = 0; 
    } else {
        kxy = cxy*log(cxy)/n;
    }        
    if(cyx == 0) { 
        kyx = 0; 
    } else {
        kyx = cyx*log(cyx)/n;
    }        
    return std::min(1-(kx-kxy)/kx, 1-(ky-kyx)/ky);
}

#endif 
