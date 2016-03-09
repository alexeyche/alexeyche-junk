#include "protobuf.h"

#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include <fcntl.h>

namespace NDnn {

	void ReadProtoTextFromFile(const TString file, google::protobuf::Message& message) {
		int fd = open(file.c_str(), O_RDONLY);
		ENSURE(fd >= 0, "Failed to open file " << file);

		google::protobuf::io::FileInputStream fstream(fd);
	    fstream.SetCloseOnDelete(true);
		ENSURE(google::protobuf::TextFormat::Parse(&fstream, &message), "Failed to parse protobuf message from file: " << file);
	}

	void WriteProtoTextToFile(const google::protobuf::Message& message, const TString file) {
		int fd = open(file.c_str(), O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
		ENSURE(fd >= 0, "Failed to open file for write " << file);

		google::protobuf::io::FileOutputStream fstream(fd);
	    fstream.SetCloseOnDelete(true);
	    ENSURE(google::protobuf::TextFormat::Print(message, &fstream), "Failed to print protobuf message into file: " << file);
	}

	void ReadProtoText(const TString& messageStr, google::protobuf::Message& message) {
		ENSURE(google::protobuf::TextFormat::ParseFromString(messageStr, &message), "Failed to parse protobuf from string: \n" << messageStr);
	}

	TString ProtoTextToString(const google::protobuf::Message& message) {
		TString s;
		google::protobuf::TextFormat::PrintToString(message, &s);
		return s;
	}

} // namespace NDnn