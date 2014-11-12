
using namespace std;

#include <iostream>
#include <stdlib.h>

#include <snnlib/util/optionparser/opt.h>
#include <snnlib/config/constants.h>

enum  optionIndex { ARG_UNKNOWN, ARG_HELP, ARG_CONSTANTS};
const option::Descriptor usage[] =
{
 {ARG_UNKNOWN, 0, "", "",Arg::None, "USAGE: example [options]\n\n"
                                        "Options:" },
 {ARG_HELP, 0,"h", "help",Arg::None, "  --help  \tPrint usage and exit." },
 {ARG_CONSTANTS, 0,"c","constants",Arg::NonEmpty, "  --constants, -c  \tConstants filename." },
 {ARG_UNKNOWN, 0, "", "",Arg::None, "\nExamples:\n"
                               "% Need to fill %" },
 {0,0,0,0,0,0}
};

void parseOptions(option::Option* options, option::Stats &stats,  int argc, char **argv) {
    option::Option* buffer  = new option::Option[stats.buffer_max];
    option::Parser parse(usage, argc, argv, options, buffer);
    if (parse.error())
        exit(1);
  
    if (options[ARG_HELP]) { // || argc == 0) {
        option::printUsage(cout, usage);
        exit(0);
    }
    if(options[ARG_CONSTANTS].count() != 1) {
        cerr << "Inappropriate constants filename argument\n";
        exit(1);
    }
    for (option::Option* opt = options[ARG_UNKNOWN]; opt; opt = opt->next()) {
      cerr << "Unknown option: " << string(opt->name,opt->namelen) << "\n";
    }
    if(options[ARG_UNKNOWN].count() > 0) exit(1);

    for (int i = 0; i < parse.nonOptionsCount(); ++i) {
      cerr << "Non-options arguments are not supported (" << i << ": " << parse.nonOption(i) << ")\n";
    }
    if(parse.nonOptionsCount()>0) exit(1);

    delete[] buffer;
}


int main(int argc, char **argv) {
    argc-=(argc>0); argv+=(argc>0); // skip program name argv[0] if present
    option::Stats  stats(usage, argc, argv);
    option::Option* options = new option::Option[stats.options_max];
    parseOptions(options, stats, argc, argv); 

    Constants c = Constants(options[ARG_CONSTANTS].arg);
    
    cout << c;
    
    delete[] options;
}
