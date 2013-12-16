
#include <sim/core.h>
#include <sim/data/load.h>

#include "lz78.h"

#include <iostream>
#include <iterator>
#include <vector>

PROGRAM_INFO("LZW_SD", "Lempel-Ziv'78 spike distance calculator"); 
PARAM_STRING("input", "input file", "i","");
PARAM_STRING("input-compare", "second input file", "c","");
PARAM_DOUBLE("T0", "T0", "", 0.0);
PARAM_DOUBLE("Tmax", "tmax", "", 750);
PARAM_DOUBLE("dt", "dt", "", 0.5);

int main(int argc, char **argv) {
    CLI::ParseCommandLine(argc, argv);
    const std::string input = CLI::GetParam<std::string>("input");
    const std::string input_comp = CLI::GetParam<std::string>("input-compare");
    const double T0 = CLI::GetParam<double>("T0");
    const double Tmax = CLI::GetParam<double>("Tmax");
    const double dt = CLI::GetParam<double>("dt");

    mat inp, inp_c;
    data::Load(input, inp, false, true);
    data::Load(input_comp, inp_c, false, true);
    if(inp.n_rows != inp_c.n_rows) {
        Log::Fatal << "error\n";
        return -1;
    }
    double dist = 0;
    for(size_t ri=0; ri<inp.n_rows; ri++) {
        TSpikeTimes st1,st2;
        vec r = inp.row(ri);
        vec r2 = inp_c.row(ri);
        for(size_t ri=0; ri<r.n_elem; ri++) {
            if(r(ri) != 0) {
                st1.push_back(r(ri));
            }
        }
        for(size_t ri2=0; ri2<r2.n_elem; ri2++) {
            if(r2(ri2) != 0) {
                st2.push_back(r2(ri2));
            }
        }
        if((st1.size() == 0)||(st2.size() == 0) ) {
            continue;
        }
        dist += lz78Distance(st1, st2, T0, Tmax, dt);
        Log::Info << "ni = " << ri << " dist = " << dist << "\n";
    }
    Log::Info << "D == " << dist << "\n";
    return 0;
}
