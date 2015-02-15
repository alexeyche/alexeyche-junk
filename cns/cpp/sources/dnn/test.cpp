#include <vector>
#include <iostream>
#include <sstream>
#include <string>
#include <typeinfo>

using std::vector;
using std::cout;
using std::stringstream;
using std::ostream;
using std::string;
using std::cerr;
using std::terminate;


#include "test.h"

using namespace dnn;

void test() {
    IAFConstants c;
    IAFNeuron *n = new IAFNeuron(c);
    //Stream s(cin, Stream::Text);
    //s << n->getState();

    Stream sout(cout, Stream::Text);
    sout << n->getState();
    
    TSInputConstants tsic;
    TSInput* tsi = new TSInput(tsic);
    n->setInput(tsi);

    DetermConstants dc;
    Determ *d = new Determ(dc);
    n->setActFunction(d);

    StdpConstants stdpc;
    Stdp *stdp = new Stdp(stdpc, *n);

    StaticSynapseConstants stat_syn_const;
    StaticSynapse *syn = new StaticSynapse(stat_syn_const);
    n->addSynapse(syn);

    delete syn;
    delete stdp;
    delete d;
    delete tsi;
    delete n;
}

