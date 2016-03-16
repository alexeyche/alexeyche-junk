#include "serial.h"

#include <dnn/util/protobuf.h>
#include <dnn/util/log/log.h>

namespace NDnn {

    TSerialBase::TSerialBase(ESerialMode mode)
        : Mode(mode)
    {
    }

    bool TSerialBase::IsInput() const {
        return Mode == ESerialMode::IN;
    }

    bool TSerialBase::IsOutput() const {
        return Mode == ESerialMode::OUT;
    }

    ///////////////////////////////////////////////////////////////////

    TProtoSerial::TProtoSerial(NPb::Message& message, ESerialMode mode)
        : Message(message)
        , TSerialBase(mode)
        , CurrentFieldNumber(1)
    {
        Refl = Message.GetReflection();
        Descr = Message.GetDescriptor();
    }

    void TProtoSerial::operator() (TString& v, int protoField) {
        switch (Mode) {
            case ESerialMode::IN:
            {
                v = Refl->GetString(Message, GetFieldDescr(protoField));
            }
            break;
            case ESerialMode::OUT:
            {
                Refl->SetString(&Message, GetFieldDescr(protoField), v);
            }
            break;
        }
    }

    void TProtoSerial::operator() (double& v, int protoField) {
        switch (Mode) {
            case ESerialMode::IN:
            {
                v = Refl->GetDouble(Message, GetFieldDescr(protoField));
            }
            break;
            case ESerialMode::OUT:
            {
                Refl->SetDouble(&Message, GetFieldDescr(protoField), v);
            }
            break;
        }
    }

    void TProtoSerial::operator() (ui32& v, int protoField) {
        switch (Mode) {
            case ESerialMode::IN:
            {
                v = Refl->GetUInt32(Message, GetFieldDescr(protoField));
            }
            break;
            case ESerialMode::OUT:
            {
                Refl->SetUInt32(&Message, GetFieldDescr(protoField), v);
            }
            break;
        }
    }

    void TProtoSerial::operator() (bool& v, int protoField) {
        switch (Mode) {
            case ESerialMode::IN:
            {
                v = Refl->GetBool(Message, GetFieldDescr(protoField));
            }
            break;
            case ESerialMode::OUT:
            {
                Refl->SetBool(&Message, GetFieldDescr(protoField), v);
            }
            break;
        }
    }

#ifdef LA_SYSTEM
    void TProtoSerial::operator() (TVector<double>& v, int protoField) {
       TVectorD laVec = NLa::StdToVec(v);
       (*this)(laVec, protoField);
       v = NLa::VecToStd(laVec);
    }

    void TProtoSerial::operator() (TVectorD& v, int protoField) {
       switch (Mode) {
           case ESerialMode::IN:
           {
               const NDnnProto::TVectorD* vecProto = GetMessage<NDnnProto::TVectorD>(protoField);
               v = TVectorD(vecProto->x_size());
               for (size_t vecIdx=0; vecIdx < v.size(); ++vecIdx) {
                   v(vecIdx) = vecProto->x(vecIdx);
               }
           }
           break;
           case ESerialMode::OUT:
           {
               NDnnProto::TVectorD* vecProto = GetEmbedMutMessage<NDnnProto::TVectorD>(protoField);
               for (const auto& val: v) {
                   vecProto->add_x(val);
               }
           }
           break;
       }
    }

    void TProtoSerial::operator() (TMatrixD& m, int protoField) {
       switch (Mode) {
           case ESerialMode::IN:
           {
               const NDnnProto::TMatrixD* mat = GetMessage<NDnnProto::TMatrixD>(protoField);
               m = TMatrixD(mat->n_rows(), mat->n_cols());
               for (size_t rowIdx=0; rowIdx < m.n_rows; ++rowIdx) {
                   const NDnnProto::TVectorD& row = mat->row(rowIdx);
                   for (size_t colIdx=0; colIdx < m.n_cols; ++colIdx) {
                       m(rowIdx, colIdx) = row.x(colIdx);
                   }
               }
           }
           break;
           case ESerialMode::OUT:
           {
               NDnnProto::TMatrixD* mat = GetEmbedMutMessage<NDnnProto::TMatrixD>(protoField);
               mat->set_n_rows(m.n_rows);
               mat->set_n_cols(m.n_cols);
               for (size_t rowIdx=0; rowIdx < m.n_rows; ++rowIdx) {
                   NDnnProto::TVectorD* row = mat->add_row();
                   for (size_t colIdx=0; colIdx < m.n_cols; ++colIdx) {
                       row->add_x(m(rowIdx, colIdx));
                   }
               }
           }
           break;
       }
    }
#endif

    void TProtoSerial::operator() (NPb::Message& m, int protoField) {
        // L_DEBUG << "Serial process of message " << m.GetTypeName() << " as " << protoField << " field number in " << Message.GetTypeName();
        auto* fd = GetFieldDescr(protoField);
        switch (Mode) {
            case ESerialMode::IN:
            {
                if (fd->is_repeated()) {
                    auto* rfPtr = Refl->MutableRepeatedPtrField<NPb::Message>(&Message, fd);
                    m.CopyFrom(rfPtr->Get(0));
                    rfPtr->DeleteSubrange(0, 1);
                } else {
                    m.CopyFrom(Refl->GetMessage(Message, fd));
                }
            }
            break;
            case ESerialMode::OUT:
            {
                NPb::Message* messageField;
                if (fd->is_repeated()) {
                    messageField = Refl->AddMessage(&Message, fd);
                } else {
                    messageField = Refl->MutableMessage(&Message, fd);
                }
                messageField->CopyFrom(m);
            }
            break;
        }
    }
    
    void TProtoSerial::operator() (IMetaProtoSerial& v, int protoField) {
        v.SerialProcess(*this);
    }

    const NPb::FieldDescriptor* TProtoSerial::GetFieldDescr(int protoField) {
        const auto* fd = Descr->FindFieldByNumber(protoField);
        ENSURE(fd, "Can't find field number " << protoField << " in message " << Message.GetTypeName());
        return fd;
    }

    /////////////////////////////////////////////////////////////////////////


    TSerialStream::TSerialStream(std::ostream& ostr) 
        : TSerialBase(ESerialMode::OUT) 
        , OStr(&ostr)
    {
    }

    TSerialStream::TSerialStream(std::istream& istr)
        : TSerialBase(ESerialMode::IN) 
        , IStr(&istr)
    {
    } 

    void TSerialStreamProtoBin::operator() (ISerialStream& v) {
        if (IsInput()) {
            TSerialStreamProtoBin str(*IStr);
            v.SerialProcess(str);    
        } else {
            TSerialStreamProtoBin str(*OStr);
            v.SerialProcess(str);    
        }
        
    }

    void TSerialStreamProtoTxt::operator() (ISerialStream& v) {
        if (IsInput()) {
            TSerialStreamProtoTxt str(*IStr);
            v.SerialProcess(str);    
        } else {
            TSerialStreamProtoTxt str(*OStr);
            v.SerialProcess(str);    
        }
    }

    TSerialStreamProtoBin::TSerialStreamProtoBin(std::ostream& ostr) 
        : TSerialStream(ostr)
    {
    }

    TSerialStreamProtoBin::TSerialStreamProtoBin(std::istream& istr) 
        : TSerialStream(istr)
    {
    }

    TSerialStreamProtoTxt::TSerialStreamProtoTxt(std::ostream& ostr) 
        : TSerialStream(ostr)
    {
    }

    TSerialStreamProtoTxt::TSerialStreamProtoTxt(std::istream& istr) 
        : TSerialStream(istr)
    {
    }

    void TSerialStreamProtoBin::DeserializeProtoFromString(const TString& s, NPb::Message& message) {
        ENSURE(message.ParseFromString(s), "Failed to deserialize message");
    }

    void TSerialStreamProtoBin::SerializeProtoToString(const NPb::Message& message, TString& s) {
        ENSURE(message.SerializeToString(&s), "Failed to serialize message");
    }


    void TSerialStreamProtoTxt::DeserializeProtoFromString(const TString& s, NPb::Message& message) {
        ReadProtoText(s, message);
    }

    void TSerialStreamProtoTxt::SerializeProtoToString(const NPb::Message& message, TString& s) {
        s = ProtoTextToString(message);
    }




} // namespace NDnn
