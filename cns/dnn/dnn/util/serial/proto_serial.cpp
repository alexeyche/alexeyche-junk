#include "proto_serial.h"
#include "meta_proto_serial.h"

namespace NDnn {

    TProtoSerial::TProtoSerial(NPb::Message& message, ESerialMode mode)
        : Message(message)
        , TSerialBase(mode)
        , CurrentFieldNumber(1)
    {
        Refl = Message.GetReflection();
        Descr = Message.GetDescriptor();
    }

    #define CHECK_FIELD() \
        if ((!HasField(protoField)) && IsInput()) { \
            return false;  \
        } \

    bool TProtoSerial::operator() (TString& v, int protoField) {
        CHECK_FIELD();
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
        return true;
    }

    bool TProtoSerial::operator() (double& v, int protoField) {
        CHECK_FIELD();

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
        return true;
    }

    bool TProtoSerial::operator() (ui32& v, int protoField) {
        CHECK_FIELD();

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
        return true;
    }

    bool TProtoSerial::operator() (int& v, int protoField) {
        CHECK_FIELD();

        switch (Mode) {
            case ESerialMode::IN:
            {
                v = Refl->GetInt32(Message, GetFieldDescr(protoField));
            }
            break;
            case ESerialMode::OUT:
            {
                Refl->SetInt32(&Message, GetFieldDescr(protoField), v);
            }
            break;
        }
        return true;
    }

    bool TProtoSerial::operator() (bool& v, int protoField) {
        CHECK_FIELD();

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
        return true;
    }

#ifdef LA_SYSTEM
    bool TProtoSerial::operator() (TVector<double>& v, int protoField) {
        CHECK_FIELD();

        TVectorD laVec = NLa::StdToVec(v);
        (*this)(laVec, protoField);
        v = NLa::VecToStd(laVec);
        return true;
    }

    bool TProtoSerial::operator() (TVectorD& v, int protoField) {
        CHECK_FIELD();

        switch (Mode) {
            case ESerialMode::IN:
            {
                const NDnnProto::TVectorD* vecProto = GetEmbedMessage<NDnnProto::TVectorD>(protoField);
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
        return true;
    }

    bool TProtoSerial::operator() (TMatrixD& m, int protoField) {
        CHECK_FIELD();

        switch (Mode) {
            case ESerialMode::IN:
            {
                const NDnnProto::TMatrixD* mat = GetEmbedMessage<NDnnProto::TMatrixD>(protoField);
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
        return true;
    }
#endif

    bool TProtoSerial::SerialRepeated(double& v, int idx, int protoField) {
        CHECK_FIELD();

        auto* fd = GetFieldDescr(protoField);
        switch (Mode) {
            case ESerialMode::IN:
            {
                v = Refl->GetRepeatedDouble(Message, fd, idx);
            }
            break;
            case ESerialMode::OUT:
            {
                Refl->AddDouble(&Message, fd, v);
            }
            break;
        }
        return true;
    }

    bool TProtoSerial::SerialRepeated(NPb::Message& m, int idx, int protoField) {
        CHECK_FIELD();

        switch (Mode) {
            case ESerialMode::IN:
            {
                const auto& messageField = Refl->GetRepeatedMessage(Message, GetFieldDescr(protoField), idx);
                m.CopyFrom(messageField);
            }
            break;
            case ESerialMode::OUT:
            {
                NPb::Message* messageField = Refl->AddMessage(&Message, GetFieldDescr(protoField));
                messageField->CopyFrom(m);
            }
            break;
        }
        return true;
    }

    bool TProtoSerial::operator() (NPb::Message& m, int protoField) {
        CHECK_FIELD();

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
        return true;
    }

    bool TProtoSerial::operator() (IMetaProtoSerial& v, int protoField, bool newMessage) {
        CHECK_FIELD();

        NPb::Message* message = GetEmbedMutMessage<NPb::Message>(protoField, newMessage);
        TMetaProtoSerial serial(*message, protoField, Mode);
        v.SerialProcess(serial);
        if (newMessage && IsInput()) {
            auto* fd = GetFieldDescr(protoField);
            ENSURE(fd->is_repeated(), "New message on non repeated field");
            auto* rfPtr = Refl->MutableRepeatedPtrField<NPb::Message>(&Message, fd);
            rfPtr->DeleteSubrange(0, 1);
        }
        return true;
    }

    const NPb::FieldDescriptor* TProtoSerial::GetFieldDescr(int protoField) {
        const auto* fd = Descr->FindFieldByNumber(protoField);
        ENSURE(fd, "Can't find field number " << protoField << " in message " << Message.GetTypeName());
        return fd;
    }

    bool TProtoSerial::HasField(int protoField) {
        auto *fd = GetFieldDescr(protoField);
        if (fd->is_repeated()) {
            return Refl->FieldSize(Message, fd) != 0;
        }
        return Refl->HasField(Message, fd);
    }

    void TProtoSerial::DuplicateSingleRepeated(int protoField, ui32 size, bool allow_zero) {
        if (IsOutput()) return;

        auto* fd = GetFieldDescr(protoField);
        ENSURE(fd->is_repeated(), "Can't duplicate non repeated field");
        ui32 actual_size = Refl->FieldSize(Message, fd);
        ENSURE((actual_size > 0) || allow_zero, "Can't duplicate field without elements, zero are not allowed");
        if ((actual_size == 0) || (actual_size == size)) {
            return;
        }
        if (actual_size != 1) {
            L_DEBUG << "Size of repeated field must equal to " << size << " or be 1 so we can duplicate this message " << size << " times for you (now it's " << actual_size << "), so we skipping duplicating";
            return;
        }

        L_DEBUG << "Duplicating " << fd->name() << " " << size << " times";
        auto* baseMessage = Refl->MutableRepeatedPtrField<NPb::Message>(&Message, fd)->Mutable(0);
        while (Refl->FieldSize(Message, fd) < size) {
            NPb::Message* newMessage = Refl->AddMessage(&Message, fd);
            newMessage->CopyFrom(*baseMessage);
        }
    }

    #undef CHECK_FIELD


} // namespace NDnn