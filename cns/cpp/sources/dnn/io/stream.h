#pragma once

#include <dnn/core.h>


namespace dnn {

template <typename T>
class Serializable;

class Stream {
public:
    enum Repr { Binary, Text };
    Stream(istream &str, Repr _r = Binary) : _input_str(&str), r(_r), _output_str(nullptr) {} 
    Stream(ostream &str, Repr _r = Binary) : _output_str(&str), r(_r), _input_str(nullptr) {} 
    bool isOutput() {
        if(_output_str) return true;
        return false;
    }
    bool isInput() {
        if(_input_str) return true;
        return false;
    }
    istream& getInputStream() {
        if(_input_str) return *_input_str;
        cerr << "Stream is wrongly opened or used\n";
        terminate();
    }
    ostream& getOutputStream() {
        if(_output_str) return *_output_str;
        cerr << "Stream is wrongly opened or used\n";
        terminate();
    }
    template<typename T>
    Stream& operator << (Serializable<T> &v);
    
    Repr getRepr() {
        return r;
    }

private:
    istream *_input_str;    
    ostream *_output_str;    
    Repr r;
};


}

#include <dnn/io/serialize.h>

namespace dnn {

template <typename T>
Stream& Stream::operator << (Serializable<T> &v) {
    v.processStream(*this);
    return *this;
}



}    