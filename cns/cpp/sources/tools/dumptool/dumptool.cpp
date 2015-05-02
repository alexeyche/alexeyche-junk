
#include <iostream>
#include <stdlib.h>
#include <fstream>

#include <dnn/core.h>
#include <dnn/util/option_parser.h>
#include <dnn/io/stream.h>
#include <dnn/util/util.h>


using namespace dnn;

const char * usage = R"USAGE(
Tool for dumping protobins.
    --input, -i   input protobin file
    --help,  -h   for this menu
)USAGE";

int main(int argc, char **argv) {
    if(argc == 1) {
        cout << usage;
        return 0; 
    }
    OptionParser optp(argc, argv);
    
    string input_file;
    bool need_help;
    optp.option("--input", "-i", input_file, true);
    optp.option("--help", "-h", input_file, false, true);
    if(need_help) {
        cout << usage;
        return 0;
    }

    ifstream ff(input_file);
    Stream s(ff, Stream::Binary);

    while(true) {
        vector<ProtoMessage> messages = s.readObjectProtos();
        if(messages.size() == 0) break;
        for(auto &m : messages) {
            string type = m->GetTypeName(); 
            vector<string> type_subs = split(type, '.');
            assert(type_subs.size() == 2);
            cout << type_subs[1] << " {\n";
            string ds = m->DebugString();
            string acc;
            vector<string> lines = split(ds, '\n');
            for(auto &l: lines) {
                acc += "    " + l + '\n';
            }
            cout << acc;
            cout << "}\n";
        }
    }
}
