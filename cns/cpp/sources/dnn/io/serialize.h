#pragma once

#include <dnn/core.h>
#include <dnn/util/util.h>
#include <dnn/util/json.h>
#include <dnn/util/interfaced_ptr.h>
#include <dnn/base/base.h>
#include <dnn/base/factory.h>
#include <dnn/protos/base.pb.h>

#include <google/protobuf/message.h>
#include <dnn/protos/generated.pb.h>


typedef google::protobuf::Message* ProtoMessage;

namespace dnn {


class SerializableBase  : public Object {
friend class Factory;
public:
    enum ProcessMode { ProcessingInput, ProcessingOutput };
    enum EndMarker { end };

    typedef SerializableBase Self;

    SerializableBase() : mode(ProcessingOutput), header(nullptr) {

    }
    virtual ProtoMessage newProto() {
        cerr << "That shouldn't be called. This method for non protobuf Serializable classes\n";
        terminate();
    }

    virtual ~SerializableBase() {
        for(auto &m: messages) {
            delete m;
        }
    }

    virtual void serial_process() = 0;
    virtual const string name() const = 0;

    static ProtoMessage copyM(ProtoMessage m) {
        ProtoMessage copy_m = m->New();
        copy_m->CopyFrom(*m);
        return copy_m;
    }

    SerializableBase& begin() {
        if(mode == ProcessingOutput) {
            while(messages.size()>0) deleteLastMessage();
            header = new Protos::ClassName;

            header->set_class_name(name());
            header->set_has_proto(false);
            header->set_size(0);

            messages.push_back(header);
        }
        return *this;
    }
    void operator << (EndMarker e) {
        if(mode == ProcessingInput) {
            deleteLastMessage();
        }
    }

    

    SerializableBase& operator << (SerializableBase &b) {
        if(mode == ProcessingOutput) {
            header->set_size(header->size()+1);
            for(auto &m: b.getSerialized()) {
                addMessage(m);
            }
        } else
        if(mode == ProcessingInput) {
            b.getDeserialized(messages);
            deleteLastMessage();
        }

        return *this;
    }

    template <typename T>
    SerializableBase& operator << (InterfacedPtr<T> &b) {
        if(mode == ProcessingOutput) {
            if(!b.isSet()) {
                cerr << "Failed to serialize InterfacePtr: it is without an pointer\n";
                terminate();
            }            
            (*this) << b.ref();
        } else
        if(mode == ProcessingInput) {
            
            //b.set(p);
            (*this) << b.ref();
        }
        return *this;
    }

    vector<ProtoMessage>& getSerialized() {
        mode = ProcessingOutput;
        while(messages.size()>0) deleteLastMessage();

        serial_process();
        return messages;
    }

    void getDeserialized(vector<ProtoMessage> &inp_mess) {
        mode = ProcessingInput;
        for(auto &m: inp_mess) {
            addMessage(m);
        }
        serial_process();
    }

    SerializableBase& operator << (const char *vraw) {
        return *this;
    }
    void addMessage(ProtoMessage m) {
        messages.push_back( copyM(m) );
    }

    ProtoMessage lastMessage() {
        if(messages.size() == 0) {
            cerr << "Trying to get from empty vector of messages\n";
            terminate();
        }
        return messages.back();
    }
    void deleteLastMessage() {
        if(!messages.empty()) {
            delete messages.back();
            messages.pop_back();
        }
    }
protected:
    vector<ProtoMessage> messages;
    Protos::ClassName *header;
    ProcessMode mode;
};


template <typename Proto>
class Serializable : public SerializableBase {
public:
    #define ASSERT_FIELDS() \
    if((!messages.size() != 0)||(!field_descr)) {\
        cerr << "Wrong using of Serializable class.\n"; \
        terminate(); \
    }\

    typedef Serializable<Proto> Self;

    const string name() const {
        Proto _fake_m;
        vector<string> spl = split(_fake_m.GetTypeName(), '.');
        if(spl[0] != "Protos") {
            cerr << "Expection Protos:: typename\n";
            terminate();
        }
        string ret;
        for(size_t i=1; i<spl.size(); ++i) {
            ret += spl[i];
        }
        return ret;
    }

    ProtoMessage newProto() {
        return new Proto;
    }

    Serializable& operator << (const char *vraw) {
        if(messages.size() == 0) {
            cerr << "Serialaling without begin()\n";
            terminate();
        }

        string v = string(vraw);
        if(trimC(v) == ",") return *this;

        vector<string> v_spl = split(v, ':');
        string fname = v_spl[0];
        trim(fname);
        
        //cout << messages.size() << "\n";
        //cout << "Filling fname " << fname << " (" << lastMessage()->GetTypeName()  << ")\n";
        const google::protobuf::Descriptor* descriptor = lastMessage()->GetDescriptor();
        field_descr = descriptor->FindFieldByName(fname);

        if(!field_descr) {
            cerr << "Can't find proto field by name " << fname << "\n";
            terminate();
        }
        return *this;
    }
    Serializable& operator << (double &v) {
        ASSERT_FIELDS()
        if(mode == ProcessingOutput) {
            lastMessage()->GetReflection()->SetDouble(lastMessage(), field_descr, v);
        } else {
            v = lastMessage()->GetReflection()->GetDouble(*lastMessage(), field_descr);
        }
        return *this;
    }

    Serializable& operator << (bool &v) {
        ASSERT_FIELDS()
        if(mode == ProcessingOutput) {
            lastMessage()->GetReflection()->SetBool(lastMessage(), field_descr, v);
        } else {
            v = lastMessage()->GetReflection()->GetBool(*lastMessage(), field_descr);
        }
        return *this;
    }

    Serializable& operator << (size_t &v) {
        ASSERT_FIELDS()
        if(mode == ProcessingOutput) {
            lastMessage()->GetReflection()->SetUInt32(lastMessage(), field_descr, v);
        } else {
            v = lastMessage()->GetReflection()->GetUInt32(*lastMessage(), field_descr);
        }
        return *this;
    }

    Serializable& operator << (string &v) {
        ASSERT_FIELDS()
        if(mode == ProcessingOutput) {
           lastMessage()->GetReflection()->SetString(lastMessage(), field_descr, v);
        } else {
           v = lastMessage()->GetReflection()->GetString(*lastMessage(), field_descr);
        }
        return *this;
    }

    void operator << (EndMarker e) {
       if(mode == ProcessingInput) {
            cout << "Deleting " << lastMessage()->GetTypeName() << "\n";
            deleteLastMessage();
        } 
    }


    Serializable& begin() {
        if(mode == ProcessingOutput) {
            while(messages.size()>0) deleteLastMessage();

            header = new Protos::ClassName;
            header->set_has_proto(true);
            header->set_class_name(name());
            header->set_size(0);
            
            messages.push_back(header);
            
            ProtoMessage mess = new Proto;
            
            messages.push_back(mess);
        }
        return *this;
    }


private:
    const google::protobuf::FieldDescriptor* field_descr;
};



}
