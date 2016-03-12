#include "meta_proto_serial.h"


namespace NDnn {


    TMetaProtoSerial::TMetaProtoSerial(NPb::Message& message, int dstFieldNumber, ESerialMode mode)
        : Message(message)
        , DstFieldNumber(dstFieldNumber)
        , TSerialBase(mode)
        , DuplicateFactor(0)
    {
    	Refl = Message.GetReflection();
        Descr = Message.GetDescriptor();
    }


	void TMetaProtoSerial::operator() (NPb::Message& m, int protoField) {
        L_DEBUG << "Serial process of message " << m.GetTypeName() << " as " << protoField << " field number in " << Message.GetTypeName();
        auto* fd = GetFieldDescr(protoField);
        switch (Mode) {
            case ESerialMode::IN:
            {
                if (fd->is_repeated()) {
                    auto* rfPtr = Refl->MutableRepeatedPtrField<NPb::Message>(&Message, fd);
                    m.CopyFrom(rfPtr->Get(0));
                } else {
                    m.CopyFrom(Refl->GetMessage(Message, fd));
                }
            }
            break;
            case ESerialMode::OUT:
            {
                NPb::Message* messageField;
                if (fd->is_repeated()) {
                	auto* rfPtr = Refl->MutableRepeatedPtrField<NPb::Message>(&Message, fd);
                    messageField = rfPtr->Mutable(0);
                } else {
                    messageField = Refl->MutableMessage(&Message, fd);
                }
                ENSURE(messageField->GetTypeName() == m.GetTypeName(), "Message types are not equal, expecting " << m.GetTypeName() << " at " << protoField << " field number, got " << messageField->GetTypeName());
                messageField->CopyFrom(m);
            }
            break;
        }
    }
    void TMetaProtoSerial::operator() (IMetaProtoSerial& v) {
        L_DEBUG << "MetaProto " << (Mode == ESerialMode::IN ? "IN" : "OUT") << ": Serial process of meta entitty";
        
        v.SerialProcess(*this);

        L_DEBUG << "MetaProto " << (Mode == ESerialMode::IN ? "IN" : "OUT") << ": Serial process done";
    }   

    void TMetaProtoSerial::DuplicateSingleRepeated(ui32 duplicateFactor) {
        if (IsOutput()) return;

        TProtoSerial serial(Message, Mode);
        for (int fi=0; fi < Descr->field_count(); ++fi) {
            auto* fd = Descr->field(fi);
            if (fd->is_repeated()) {
                serial.DuplicateSingleRepeated(fd->number(), duplicateFactor);
            }
        }
    }

    const NPb::FieldDescriptor* TMetaProtoSerial::GetFieldDescr(int protoField) {
        const auto* fd = Descr->FindFieldByNumber(protoField);
        ENSURE(fd, "Can't find field number " << protoField << " in message " << Message.GetTypeName());
        return fd;
    }

} // namespace NDnn