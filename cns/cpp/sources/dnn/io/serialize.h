#pragma once

#include <dnn/core.h>
#include <dnn/util/util.h>
#include <dnn/util/json.h>
#include <dnn/util/interfaced_ptr.h>
#include <dnn/base/base.h>
#include <dnn/base/factory.h>

#include <google/protobuf/message.h>
#include <dnn/protos/generated.pb.h>


typedef google::protobuf::Message* ProtoMessage;
typedef pair<string, ProtoMessage> NamedMessage;

namespace dnn {


class SerializableBase  : public Object {
friend class Factory;
public:
    enum ProcessMode { ProcessingInput, ProcessingOutput };
    enum EndMarker { End };


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

    void operator << (EndMarker e) {}

    SerializableBase& begin() {
        cout << "BEGIN BASE\n";
        return *this;
    }
    EndMarker end() {
        terminate();
        cout << "END BASE\n";
        if(mode == ProcessingInput) {
            cout << "Deleting " << lastMessage()->GetTypeName() << "\n";
            deleteLastMessage();
        }
        return End;
    }



    SerializableBase& operator << (SerializableBase &b) {
        if(mode == ProcessingOutput) {
            for(auto &m: b.getSerialized()) {
                addMessage(m.first, m.second);
            }
        } else
        if(mode == ProcessingInput) {
            b.getDeserialized(messages);
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
            SerializableBase* o = Factory::inst().createObject(lastMessageName());
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
        messages = inp_mess;
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
        if(messages.size() > 0) {
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
    if((!reflection)||(!messages.size() != 0)||(!field_descr)) {\
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
        cout << "Filling fname " << fname << "\n";
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
            reflection->SetDouble(lastMessage(), field_descr, v);
        } else {
            v = reflection->GetDouble(*lastMessage(), field_descr);
        }
        return *this;
    }

    Serializable& operator << (bool &v) {
        ASSERT_FIELDS()
        if(mode == ProcessingOutput) {
            reflection->SetBool(lastMessage(), field_descr, v);
        } else {
            v = reflection->GetBool(*lastMessage(), field_descr);
        }
        return *this;
    }

    Serializable& operator << (size_t &v) {
        ASSERT_FIELDS()
        if(mode == ProcessingOutput) {
            reflection->SetUInt32(lastMessage(), field_descr, v);
        } else {
            v = reflection->GetUInt32(*lastMessage(), field_descr);
        }
        return *this;
    }

    Serializable& operator << (string &v) {
        ASSERT_FIELDS()
        if(mode == ProcessingOutput) {
           reflection->SetString(lastMessage(), field_descr, v);
        } else {
           v = reflection->GetString(*lastMessage(), field_descr);
        }
        return *this;
    }

    void operator << (EndMarker e) {}


    Serializable& begin() {
        cout << "BEGIN\n";
        if(mode == ProcessingOutput) {
            ProtoMessage mess = new Proto;
            reflection = mess->GetReflection();
            addMessage(name(), mess);
        }
        return *this;
    }


private:
    const google::protobuf::FieldDescriptor* field_descr;
    const google::protobuf::Reflection* reflection;
};



}
