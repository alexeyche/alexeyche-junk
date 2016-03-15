
template <typename T>
void DumpEntity(TBinSerial& serial) {
	typename T::TProto pb;
	serial.ReadProtobufMessage(pb);
    std::cout << pb.DebugString();
}