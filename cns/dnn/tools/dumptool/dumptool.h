
template <typename T>
void DumpEntity(TBinSerial& serial) {
	typename T::TProto pb;
	serial.ReadProtobufMessage(pb);
    std::cout << pb.DebugString();
}

template <typename T>
void DumpEntities(TBinSerial& serial) {
	typename T::TProto pb;
	while (serial.ReadProtobufMessage(pb)) {
		std::cout << pb.DebugString();	
	}
    
}