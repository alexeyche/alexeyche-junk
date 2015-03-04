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
typedef pair<string, ProtoMessage> NamedMessage;

namespace dnn {


class SerializableBase  : public Object {
friend class Factory;
public:
    enum ProcessMode { ProcessingInput, ProcessingOutput };
    enum EndMarker { end };

    typedef SerializableBase Self;

    SerializableBase() : mode(ProcessingOutput) {

    }
    virtual ProtoMessage newProto() {
        cerr << "That shouldn't be called. This method for non protobuf Serializable classes\n";
        terminate();
    }

    virtual ~SerializableBase() {
        for(auto &m: messages) {
            delete m.second;
        }
    }

    virtual void serial_process() = 0;
    virtual const string name() const = 0;

    static ProtoMessage copyM(ProtoMessage m) {
        ProtoMessage copy_m = m->New();
        copy_m->CopyFrom(*m);
        return copy_m;
    }

    void operator << (EndMarker e) {
        if(mode == ProcessingInput) {
            cout << "Deleting " << lastMessage()->GetTypeName() << "\n";
            deleteLastMessage();
        }
    }

    SerializableBase& begin() {
        return *this;
    }

    SerializableBase& operator << (SerializableBase &b) {
        if(mode == ProcessingOutput) {
            for(auto &m: b.getSerialized()) {
                addMessage(m.first, m.second);
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
            Protos::ClassName *cl = new Protos::ClassName;
            cl->set_class_name(b.ref().name());
            messages.push_back(NamedMessage("ClassName", cl));
            (*this) << b.ref();
        } else
        if(mode == ProcessingInput) {
            cout << lastMessage()->GetTypeName() << "\n";
            Protos::ClassName *cl = static_cast<Protos::ClassName*>(lastMessage());
            
            SerializableBase* o = Factory::inst().createObject(cl->class_name());
            deleteLastMessage();
            
            T* p = dynamic_cast<T*>(o);
            if(!p) {
                cerr << "Failed to cast " << o->name() << "\n";
                terminate();
            }
            b.set(p);
            (*this) << b.ref();
        }
        return *this;
    }

    vector<NamedMessage>& getSerialized() {
        mode = ProcessingOutput;
        while(messages.size()>0) deleteLastMessage();

        serial_process();
        return messages;
    }

    void getDeserialized(vector<NamedMessage> &inp_mess) {
        mode = ProcessingInput;
        for(auto &m: inp_mess) {
            messages.push_back(NamedMessage(m.first, copyM(m.second)));
        }
        serial_process();
    }

    SerializableBase& operator << (const char *vraw) {
        return *this;
    }
    void addMessage(string name, ProtoMessage m) {
        messages.push_back( NamedMessage(name, copyM(m)) );
    }

    ProtoMessage lastMessage() {
        if(messages.size() == 0) {
            cerr << "Trying to get from empty vector of messages\n";
            terminate();
        }
        return messages.back().second;
    }
    const string& lastMessageName() {
        if(messages.size() == 0) {
            cerr << "Trying to get from empty vector of messages\n";
            terminate();
        }
        return messages.back().first;
    }
    void deleteLastMessage() {
        if(!messages.empty()) {
            delete messages.back().second;
            messages.pop_back();
        }
    }
protected:
    vector<NamedMessage> messages;
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
        
        cout << messages.size() << "\n";
        cout << "Filling fname " << fname << " (" << lastMessage()->GetTypeName()  << ")\n";
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
            ProtoMessage mess = new Proto;
            addMessage(name(), mess);
        }
        return *this;
    }


private:
    const google::protobuf::FieldDescriptor* field_descr;
};



}
