    // TSerialStream::TSerialStream(std::ostream& ostr) 
    //     : TSerialBase(ESerialMode::OUT) 
    //     , OStr(&ostr)
    // {
    // }

    // TSerialStream::TSerialStream(std::istream& istr)
    //     : TSerialBase(ESerialMode::IN) 
    //     , IStr(&istr)
    // {
    // } 

    // void TSerialStreamProtoBin::operator() (ISerialStream& v) {
    //     if (IsInput()) {
    //         TSerialStreamProtoBin str(*IStr);
    //         v.SerialProcess(str);    
    //     } else {
    //         TSerialStreamProtoBin str(*OStr);
    //         v.SerialProcess(str);    
    //     }
        
    // }

    // void TSerialStreamProtoTxt::operator() (ISerialStream& v) {
    //     if (IsInput()) {
    //         TSerialStreamProtoTxt str(*IStr);
    //         v.SerialProcess(str);    
    //     } else {
    //         TSerialStreamProtoTxt str(*OStr);
    //         v.SerialProcess(str);    
    //     }
    // }

    // TSerialStreamProtoBin::TSerialStreamProtoBin(std::ostream& ostr) 
    //     : TSerialStream(ostr)
    // {
    // }

    // TSerialStreamProtoBin::TSerialStreamProtoBin(std::istream& istr) 
    //     : TSerialStream(istr)
    // {
    // }

    // TSerialStreamProtoTxt::TSerialStreamProtoTxt(std::ostream& ostr) 
    //     : TSerialStream(ostr)
    // {
    // }

    // TSerialStreamProtoTxt::TSerialStreamProtoTxt(std::istream& istr) 
    //     : TSerialStream(istr)
    // {
    // }

    // void TSerialStreamProtoBin::DeserializeProtoFromString(const TString& s, NPb::Message& message) {
    //     ENSURE(message.ParseFromString(s), "Failed to deserialize message");
    // }

    // void TSerialStreamProtoBin::SerializeProtoToString(const NPb::Message& message, TString& s) {
    //     ENSURE(message.SerializeToString(&s), "Failed to serialize message");
    // }


    // void TSerialStreamProtoTxt::DeserializeProtoFromString(const TString& s, NPb::Message& message) {
    //     ReadProtoText(s, message);
    // }

    // void TSerialStreamProtoTxt::SerializeProtoToString(const NPb::Message& message, TString& s) {
    //     s = ProtoTextToString(message);
    // }

