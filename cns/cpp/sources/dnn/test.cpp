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
    IAFConstants *c = Factory::inst().createObject<IAFConstants>("IAFConstants");
    IAFNeuron *n = new IAFNeuron();
    n->setConstants(*c);
    Stream sout(cout, Stream::Text);
    sout << *n;

    delete n;
}

