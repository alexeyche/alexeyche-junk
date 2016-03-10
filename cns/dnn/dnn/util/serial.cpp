#include "serial.h"

#include <dnn/util/protobuf.h>

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



    TProtoSerial::TProtoSerial(google::protobuf::Message& message, ESerialMode mode)
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
                v = Refl->GetString(Message, Descr->FindFieldByNumber(protoField));
            }
            break;
            case ESerialMode::OUT:
            {
                Refl->SetString(&Message, Descr->FindFieldByNumber(protoField), v);
            }
            break;
        }
    }

    void TProtoSerial::operator() (double& v, int protoField) {
        switch (Mode) {
            case ESerialMode::IN:
            {
                v = Refl->GetDouble(Message, Descr->FindFieldByNumber(protoField));
            }
            break;
            case ESerialMode::OUT:
            {
                Refl->SetDouble(&Message, Descr->FindFieldByNumber(protoField), v);
            }
            break;
        }
    }

    void TProtoSerial::operator() (ui32& v, int protoField) {
        switch (Mode) {
            case ESerialMode::IN:
            {
                v = Refl->GetUInt32(Message, Descr->FindFieldByNumber(protoField));
            }
            break;
            case ESerialMode::OUT:
            {
                Refl->SetUInt32(&Message, Descr->FindFieldByNumber(protoField), v);
            }
            break;
        }
    }

    void TProtoSerial::operator() (bool& v, int protoField) {
        switch (Mode) {
            case ESerialMode::IN:
            {
                v = Refl->GetBool(Message, Descr->FindFieldByNumber(protoField));
            }
            break;
            case ESerialMode::OUT:
            {
                Refl->SetBool(&Message, Descr->FindFieldByNumber(protoField), v);
            }
            break;
        }
    }

    //void TProtoSerial::operator() (TVector<double>& v, int protoField) {
    //    TVectorD laVec = NLa::StdToVec(v);
    //    (*this)(laVec, protoField);
    //    v = NLa::VecToStd(laVec);
    //}

    //void TProtoSerial::operator() (TVectorD& v, int protoField) {
    //    switch (Mode) {
    //        case ESerialMode::IN:
    //        {
    //            const NDnnProto::TVectorD* vecProto = GetMessage<NDnnProto::TVectorD>(protoField);
    //            v = TVectorD(vecProto->x_size());
    //            for (size_t vecIdx=0; vecIdx < v.size(); ++vecIdx) {
    //                v(vecIdx) = vecProto->x(vecIdx);
    //            }
    //        }
    //        break;
    //        case ESerialMode::OUT:
    //        {
    //            NDnnProto::TVectorD* vecProto = GetMutMessage<NDnnProto::TVectorD>(protoField);
    //            for (const auto& val: v) {
    //                vecProto->add_x(val);
    //            }
    //        }
    //        break;
    //    }
    //}

    //void TProtoSerial::operator() (TMatrixD& m, int protoField) {
    //    switch (Mode) {
    //        case ESerialMode::IN:
    //        {
    //            const NDnnProto::TMatrixD* mat = GetMessage<NDnnProto::TMatrixD>(protoField);
    //            m = TMatrixD(mat->n_rows(), mat->n_cols());
    //            for (size_t rowIdx=0; rowIdx < m.n_rows; ++rowIdx) {
    //                const NDnnProto::TVectorD& row = mat->row(rowIdx);
    //                for (size_t colIdx=0; colIdx < m.n_cols; ++colIdx) {
    //                    m(rowIdx, colIdx) = row.x(colIdx);
    //                }
    //            }
    //        }
    //        break;
    //        case ESerialMode::OUT:
    //        {
    //            NDnnProto::TMatrixD* mat = GetMutMessage<NDnnProto::TMatrixD>(protoField);
    //            mat->set_n_rows(m.n_rows);
    //            mat->set_n_cols(m.n_cols);
    //            for (size_t rowIdx=0; rowIdx < m.n_rows; ++rowIdx) {
    //                NDnnProto::TVectorD* row = mat->add_row();
    //                for (size_t colIdx=0; colIdx < m.n_cols; ++colIdx) {
    //                    row->add_x(m(rowIdx, colIdx));
    //                }
    //            }
    //        }
    //        break;
    //    }
    //}

    void TProtoSerial::operator() (google::protobuf::Message& m, int protoField) {
        switch (Mode) {
            case ESerialMode::IN:
            {
                const google::protobuf::Message& messageField = Refl->GetMessage(Message, Descr->FindFieldByNumber(protoField));
                m.CopyFrom(messageField);
            }
            break;
            case ESerialMode::OUT:
            {
                google::protobuf::Message* messageField = Refl->MutableMessage(&Message, Descr->FindFieldByNumber(protoField));
                messageField->CopyFrom(m);
            }
            break;
        }
    }

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

    void TSerialStreamProtoBin::DeserializeProtoFromString(const TString& s, google::protobuf::Message& message) {
        ENSURE(message.ParseFromString(s), "Failed to deserialize message");
    }

    void TSerialStreamProtoBin::SerializeProtoToString(const google::protobuf::Message& message, TString& s) {
        ENSURE(message.SerializeToString(&s), "Failed to serialize message");
    }


    void TSerialStreamProtoTxt::DeserializeProtoFromString(const TString& s, google::protobuf::Message& message) {
        ReadProtoText(s, message);
    }

    void TSerialStreamProtoTxt::SerializeProtoToString(const google::protobuf::Message& message, TString& s) {
        s = ProtoTextToString(message);
    }




} // namespace NDnn
