	// class TSerialStream;

 //    class ISerialStream {
 //    public:
 //        virtual void SerialProcess(TSerialStream& serial) = 0;
 //    };


 //    class TSerialStream: public TSerialBase {
 //    public:

 //        template <typename T>
 //        void operator() (IProtoSerial<T>& v) {
 //            if (IsInput()) {
 //                TString s;
 //                (*IStr) >> s;
 //                T proto;
 //                DeserializeProtoFromString(s, proto);
 //                v.Deserialize(proto);
 //            } else {
 //                T proto = v.Serialize();
 //                TString s;
 //                SerializeProtoToString(proto, s);
 //                (*OStr) << s;
 //            }
 //        }

 //        template <typename T>
 //        void operator() (const IProtoSerial<T>& v) {
 //            (*this)(const_cast<IProtoSerial<T>&>(v));
 //        }
 //        virtual void operator() (ISerialStream& v) = 0;

 //        virtual void DeserializeProtoFromString(const TString& s, NPb::Message& message) = 0;
 //        virtual void SerializeProtoToString(const NPb::Message& message, TString& s) = 0;

 //    protected:
 //        std::ostream* OStr;
 //        std::istream* IStr;
 //    };



 //    class TSerialStreamProtoBin: public TSerialStream {
 //    public:
 //        TSerialStreamProtoBin(std::ostream& ostr);
 //        TSerialStreamProtoBin(std::istream& istr);

 //        void operator() (ISerialStream& v) override final;
        
 //        void DeserializeProtoFromString(const TString& s, NPb::Message& message) override final;
 //        void SerializeProtoToString(const NPb::Message& message, TString& s) override final;
 //    };

 //    class TSerialStreamProtoTxt: public TSerialStream {
 //    public:
 //        TSerialStreamProtoTxt(std::ostream& ostr);
 //        TSerialStreamProtoTxt(std::istream& istr);

 //        void operator() (ISerialStream& v) override final;

 //        void DeserializeProtoFromString(const TString& s, NPb::Message& message) override final;
 //        void SerializeProtoToString(const NPb::Message& message, TString& s) override final;
 //    };
