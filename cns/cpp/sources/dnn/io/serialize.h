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

class Factory;

class SerializableBase  : public Object {
friend class Factory;
public:
    enum ProcessMode { ProcessingInput, ProcessingOutput };
    enum EndMarker { end };

    typedef SerializableBase Self;
    static const bool hasProto = false;
    typedef Protos::EmptyProto ProtoType;

    SerializableBase() : mode(ProcessingOutput), messages(nullptr), header(nullptr) {

    }
    virtual ProtoMessage newProto() {
        cerr << "That shouldn't be called. This method for non protobuf Serializable classes\n";
        terminate();
    }

    virtual ~SerializableBase() {
        if(mode == ProcessingOutput) {
            if(messages) clean();
        }
    }
    
    void clean() {
        for(auto &m: *messages) {
            delete m;
        }
        delete messages;
    }

    virtual void serial_process() = 0;
    virtual const string name() const = 0;

    static ProtoMessage copyM(ProtoMessage m) {
        ProtoMessage copy_m = m->New();
        copy_m->CopyFrom(*m);
        return copy_m;
    }
    static Protos::ClassName* getHeader(vector<ProtoMessage> &messages) {
        if(messages.size() == 0) {
            cerr << "Trying to get header from empty messages stack\n";
            terminate();
        }
        Protos::ClassName *head = dynamic_cast<Protos::ClassName*>(messages.back());
        if(!head) {
            cerr << "There is no header on the top of the stack\n";
            cerr << "Got " << messages.back()->GetTypeName() << "\n";
            terminate();
        }
        return head;
    }

    Protos::ClassName* getHeader() {
        if(messages) {
            return getHeader(*messages);
        }
        cerr << "Null messages stack\n";
        terminate();
    }

    SerializableBase& begin() {
        assert(messages);
        if(mode == ProcessingOutput) {
            header = new Protos::ClassName;

            header->set_class_name(name());
            header->set_has_proto(false);
            header->set_size(0);

            messages->push_back(header);
        }
        if(mode == ProcessingInput) {
            Protos::ClassName *head = getHeader();
            if(name() != head->class_name()) {
                cerr << "Error while deserializing. Wrong class name header: " << name() << " != " << head->class_name() << "\n";
                terminate();
            }
            deleteCurrentMessage();
        }
        return *this;
    }
    void operator << (EndMarker e) {
        if(mode == ProcessingInput) {
            deleteCurrentMessage();
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
            // cout << "Deserializing " << b.name() << "\n";
            b.getDeserialized(*messages);
            //cout << "deleting " << currentMessage()->GetTypeName() << "\n";
            //deleteCurrentMessage();
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
            SerializableBase *pb = Factory::inst().createObject(getHeader()->class_name());

            T* p = dynamic_cast<T*>(pb);
            if(!p) {
                cerr << name() << ": cast error while deserializing interfaced ptr, got " << pb->name() << "\n";
                terminate();
            }            
            b.set(p);
            (*this) << b.ref();
        }
        return *this;
    }

    vector<ProtoMessage>& getSerialized() {
        if((mode == ProcessingOutput) && (messages)) clean();
        mode = ProcessingOutput;
        
        messages = new vector<ProtoMessage>;


        serial_process();
        return *messages;
    }

    void getDeserialized(vector<ProtoMessage> &inp_mess) {
        mode = ProcessingInput;
        
        if(messages) clean();
        messages = &inp_mess;

        serial_process();
    }

    SerializableBase& operator << (const char *vraw) {
        return *this;
    }
    void addMessage(ProtoMessage m) {
        assert(messages);
        messages->push_back( copyM(m) );
    }

    ProtoMessage currentMessage() {
        assert(messages);
        if(messages->size() == 0) {
            cerr << "Trying to get from empty vector of messages\n";
            terminate();
        }
        return messages->back();
    }
    void deleteCurrentMessage() {
        assert(messages);
        if(!messages->empty()) {
            // cout << name() << " stack: \n\t";
            // for(size_t i=0; i<(messages->size()-1); ++i) {
            //     cout << (*messages)[i]->GetTypeName() << ", ";
            // }
            // cout << " || " << messages->back()->GetTypeName() << "\n";
            delete messages->back();
            messages->pop_back();
        }
    }
protected:
    vector<ProtoMessage> *messages;
    Protos::ClassName *header;
    ProcessMode mode;
};


template <typename Proto>
class Serializable : public SerializableBase {
public:
    #define ASSERT_FIELDS() \
    if((messages->size() == 0)||(!field_descr)) {\
        cerr << "Wrong using of Serializable class.\n"; \
        terminate(); \
    }\

    typedef Serializable<Proto> Self;
    typedef Proto ProtoType;
    static const bool hasProto = true;

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
        if(messages->size() == 0) {
            cerr << "Serialaling without begin()\n";
            terminate();
        }

        string v = string(vraw);
        if(trimC(v) == ",") return *this;

        vector<string> v_spl = split(v, ':');
        string fname = v_spl[0];
        trim(fname);
        
        //cout << messages->size() << "\n";
        // cout << "Filling fname " << fname << " (" << currentMessage()->GetTypeName()  << ")\n";
        const google::protobuf::Descriptor* descriptor = currentMessage()->GetDescriptor();
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
            currentMessage()->GetReflection()->SetDouble(currentMessage(), field_descr, v);
        } else {
            v = currentMessage()->GetReflection()->GetDouble(*currentMessage(), field_descr);
        }
        return *this;
    }

    Serializable& operator << (bool &v) {
        ASSERT_FIELDS()
        if(mode == ProcessingOutput) {
            currentMessage()->GetReflection()->SetBool(currentMessage(), field_descr, v);
        } else {
            v = currentMessage()->GetReflection()->GetBool(*currentMessage(), field_descr);
        }
        return *this;
    }

    Serializable& operator << (size_t &v) {
        ASSERT_FIELDS()
        if(mode == ProcessingOutput) {
            currentMessage()->GetReflection()->SetUInt32(currentMessage(), field_descr, v);
        } else {
            v = currentMessage()->GetReflection()->GetUInt32(*currentMessage(), field_descr);
        }
        return *this;
    }
    Serializable& operator << (int &v) {
        ASSERT_FIELDS()
        if(mode == ProcessingOutput) {
            currentMessage()->GetReflection()->SetInt32(currentMessage(), field_descr, v);
        } else {
            v = currentMessage()->GetReflection()->GetInt32(*currentMessage(), field_descr);
        }
        return *this;
    }
    Serializable& operator << (string &v) {
        ASSERT_FIELDS()
        if(mode == ProcessingOutput) {

           currentMessage()->GetReflection()->SetString(currentMessage(), field_descr, v);
        } else {
           v = currentMessage()->GetReflection()->GetString(*currentMessage(), field_descr);
        }
        return *this;
    }
    Serializable& operator << (vector<string> &v) {
        ASSERT_FIELDS()
        if(mode == ProcessingOutput) {
            for(size_t i=0; i<v.size(); ++i) {
                currentMessage()->GetReflection()->AddString(currentMessage(), field_descr, v[i]);
            }
        } else {
            size_t cur = currentMessage()->GetReflection()->FieldSize(*currentMessage(), field_descr);
            for(size_t i=0; i<cur; ++i) {
                string subv = currentMessage()->GetReflection()->GetRepeatedString(*currentMessage(), field_descr, i);
                v.push_back(subv);
            }
        }
        return *this;
    }
    Serializable& operator << (vector<size_t> &v) {
        ASSERT_FIELDS()
        if(mode == ProcessingOutput) {
            for(size_t i=0; i<v.size(); ++i) {
                currentMessage()->GetReflection()->AddUInt32(currentMessage(), field_descr, v[i]);
            }
        } else {
            size_t cur = currentMessage()->GetReflection()->FieldSize(*currentMessage(), field_descr);
            for(size_t i=0; i<cur; ++i) {
                size_t subv = currentMessage()->GetReflection()->GetRepeatedUInt32(*currentMessage(), field_descr, i);
                v.push_back(subv);
            }
        }
        return *this;
    }
    Serializable& operator << (vector<double> &v) {
        ASSERT_FIELDS()
        if(mode == ProcessingOutput) {
            for(size_t i=0; i<v.size(); ++i) {
                currentMessage()->GetReflection()->AddDouble(currentMessage(), field_descr, v[i]);
            }
        } else {
            size_t cur = currentMessage()->GetReflection()->FieldSize(*currentMessage(), field_descr);
            for(size_t i=0; i<cur; ++i) {
                double subv = currentMessage()->GetReflection()->GetRepeatedDouble(*currentMessage(), field_descr, i);
                v.push_back(subv);
            }
        }
        return *this;
    }
    void operator << (EndMarker e) {
        if(mode == ProcessingInput) {
            deleteCurrentMessage();
        }
    }


    Serializable& begin() {
        if(mode == ProcessingOutput) {
            header = new Protos::ClassName;

            header->set_class_name(name());
            header->set_has_proto(true);
            header->set_size(0);
            
            messages->push_back(header);
            
            ProtoMessage mess = new Proto;
            
            messages->push_back(mess);
        }
        if(mode == ProcessingInput) {
            Protos::ClassName *head = getHeader();
            if(name() != head->class_name()) {
                cerr << "Error while deserializing. Wrong class name header: " << name() << " != " << head->class_name() << "\n";
                terminate();
            }
            deleteCurrentMessage();
        }
        return *this;
    }


private:
    const google::protobuf::FieldDescriptor* field_descr;
};



}
