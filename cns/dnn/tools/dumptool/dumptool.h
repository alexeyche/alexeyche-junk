
template <typename T>
bool DumpEntity(std::istream& istr) {
	typename T::TProto pb;
	if (pb.ParseFromIstream(&istr)) {
		T ent;
		ent.Deserialize(pb);
        std::cout << pb.DebugString();
        return true;
	}
    return false;
}