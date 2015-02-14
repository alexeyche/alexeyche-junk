#pragma once


namespace dnn {


class SerializeStream {
public:
    SerializeStream(ostream &_dst_stream) : dst_stream(_dst_stream) {}
    virtual void put(string s) = 0;
protected:
    ostream &dst_stream;    
};

class StringSerializeStream : public SerializeStream {
public: 
    StringSerializeStream(ostream &_dst_stream) : SerializeStream(_dst_stream) {}
    void put(string s) {
        dst_stream << s << "\n";
    }
};



class OStream {
public: 
    enum EndMarker { End };
    static EndMarker end;

    OStream(SerializeStream &_serial_stream) : serial_stream(_serial_stream), count(0), started(false) {}
    
    template <typename T>
    OStream& operator << (T &v) {
        if(typeid(v).name() == typeid(EndMarker).name()) {
            serial_stream.put(ss.str());
            return *this;
        }
        
        ss << v;
    
        count++;
        if(count % 2 == 0) {
            ss << "; ";
        }
        return *this;
    }

    
private:
    bool started;
    stringstream ss;
    size_t count;

    SerializeStream &serial_stream;
};
OStream::EndMarker OStream::end = End;


}
