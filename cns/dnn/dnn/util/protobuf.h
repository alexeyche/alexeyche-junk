#pragma once

#include <google/protobuf/message.h>
#include <dnn/base/base.h>

namespace NDnn {

	void ReadProtoTextFromFile(const TString file, google::protobuf::Message& message);

    void WriteProtoTextToFile(const google::protobuf::Message& message, const TString file);

	void ReadProtoText(const TString& messageStr, google::protobuf::Message& message);

	TString ProtoTextToString(const google::protobuf::Message& message);


} // namespace NDnn