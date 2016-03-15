#pragma once

#include <google/protobuf/message.h>
#include <dnn/base/base.h>

namespace NDnn {

	void ReadProtoTextFromFile(const TString file, google::protobuf::Message& message);

    void WriteProtoTextToFile(const google::protobuf::Message& message, const TString file);

	void ReadProtoText(const TString& messageStr, google::protobuf::Message& message);

	TString ProtoTextToString(const google::protobuf::Message& message);

	EProtobufType ReadProtobufType(std::istream& str);

	void WriteProtobufType(std::ostream& str, EProtobufType type);

	enum EProtobufType {
		CONFIG_PROTO = 0,
		TIME_SERIES_PROTO = 1,
		SPIKES_LIST_PROTO = 2
	};

} // namespace NDnn