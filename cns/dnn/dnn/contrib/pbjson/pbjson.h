#include "pbjson.hpp"

namespace NDnn {
    namespace NPbJson {

        TString ProtobufToJson(const google::protobuf::Message& message) {
            rapidjson::Value::AllocatorType allocator;
            NJson::Value* json = pbjson::pb2jsonobject(&message, allocator);
            NJson::StringBuffer buffer;
            NJson::PrettyWriter<NJson::StringBuffer> writer(buffer);
            json->Accept(writer);
            TString ret = buffer.GetString();
            delete json;
            return ret;
        }

    } // namespace NPbJson
} // namespace NDnn

