

#include <snnlib/util/optionparser/opt.h>
#include <snnlib/serialize/proto_rw.h>
#include <snnlib/neurons/neuron_stat.h>
#include <snnlib/util/spikes_list.h>
#include <snnlib/base.h>
#include <snnlib/util/matrix.h>

#include "p_stat_dist.h"
#include "cross_spectrum_dist.h"
#include "mean_net_rate.h"


void printModUsage() {
    cout << "snn_proc has subprograms:\n";
    cout << "\t\tp_stat_dist -- calculate distance matrix between intensity values\n";
    cout << "\t\tcross_spectrum_dist -- calculate distance matrix based on cross spectrum metric\n";
    cout << "\t\tmean_net_rate -- get mean net rate\n";
}

int main(int argc, char **argv) {
    if(argc == 1) {
        printModUsage();
        return 1;
    }
    string mod = trimC(string(argv[1]));
    if ((mod  == "-h") || (mod == "--help")) {
        printModUsage();
        return 1;
    } else
    if(mod == "p_stat_dist") {
        p_stat_dist(--argc, ++argv);
    } else
    if(mod == "cross_spectrum_dist") {
        cross_spectrum_dist(--argc, ++argv);
    } else
    if(mod == "mean_net_rate") {
        mean_net_rate(--argc, ++argv);
    } else {
        cerr << "Failed to recognize subprogram: " << mod << "\n";
        printModUsage();
        return 1;
    }
    return 0;
}

