#include "response_builder.h"

namespace NDnn {

    TResponseBuilder& TResponseBuilder::StaticFile(TString file) {
        std::ifstream f(file);
        if(!f) {
            throw TDnnFileNotFound() << "Failed to read file " << file;
        }
        if (NStr::EndsWith(file, ".html")) {
            TString line;
            while (std::getline(f, line)) {
                std::regex e("(<!--[ ]*#include[ ]+virtual[ ]*=[ ]*\"(.*)\"[ ]*-->)");
                std::smatch match;
                if (std::regex_search(line, match, e) && match.size()>1) {
                    TString includeFile = match.str(2);
                    ENSURE(NStr::EndsWith(includeFile, ".html"), "Can't make SSI for non html files");
                    L_DEBUG << "SSI for " << includeFile;
                    StaticFile(includeFile);
                }
                Response.Body += line;
            }
        } else {
            Response.Body += TString((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        }

        DeduceMimeTypeFromFile(file);
        return *this;
    }

	void TResponseBuilder::DeduceMimeTypeFromFile(TString path) {
        if (NStr::EndsWith(path, ".css")) {
            Response.Headers.push_back(MakePair("Content-Type", "text/css"));
        } else
        if (NStr::EndsWith(path, ".html")) {
            Response.Headers.push_back(MakePair("Content-Type", "text/html"));
        } else
        if (NStr::EndsWith(path, ".js")) {
            Response.Headers.push_back(MakePair("Content-Type", "application/javascript"));
        } else
        if (NStr::EndsWith(path, ".woff")) {
            Response.Headers.push_back(MakePair("Content-Type", "application/x-font-woff"));
        } else
        if (NStr::EndsWith(path, ".ttf")) {
            Response.Headers.push_back(MakePair("Content-Type", "application/octet-stream"));
        } else
        if (NStr::EndsWith(path, ".ico")) {
            Response.Headers.push_back(MakePair("Content-Type", "image/x-icon"));
        } else {
            L_DEBUG << "Can't deduce mime type for " << path;
            Response.Headers.push_back(MakePair("Content-Type", "text/html"));
        }
    }

    TResponseBuilder& TResponseBuilder::Good() {
        Response.Code = 200;
        Response.Status = "OK";
        return *this;
    }

    TResponseBuilder& TResponseBuilder::Created() {
        Response.Code = 201;
        Response.Status = "Created";
        return *this;
    }

    TResponseBuilder& TResponseBuilder::Accepted() {
        Response.Code = 202;
        Response.Status = "Accepted";
        return *this;
    }

    TResponseBuilder& TResponseBuilder::NotFound() {
        Response.Code = 404;
        Response.Status = "Not Found";
        return *this;
    }

    TResponseBuilder& TResponseBuilder::BadRequest() {
        Response.Code = 400;
        Response.Status = "Bad Request";
        return *this;
    }

    TResponseBuilder& TResponseBuilder::InternalError() {
        Response.Code = 500;
        Response.Status = "Internal Server Error";
        return *this;
    }
    
    TResponseBuilder& TResponseBuilder::ServiceUnavailable() {
        Response.Code = 503;
        Response.Status = "Service Unavailable";
        return *this;
    }

    TString& TResponseBuilder::Body() {
        return Response.Body;
    }

    TResponseBuilder& TResponseBuilder::Body(const TString&& body) {
        Response.Body = body;
        return *this;
    }

    THttpResponse TResponseBuilder::FormResponse() {
        Response.Headers.push_back(MakePair("Content-Length", TString(NStr::TStringBuilder() << Response.Body.size())));
        Response.Headers.push_back(MakePair("Connection", "Closed"));
        Response.Headers.push_back(MakePair("Date", GetDateAndTime()));
        Response.Headers.push_back(MakePair("Server", "DnnServer"));
        Response.Headers.push_back(MakePair("LastModified", GetDateAndTime()));
        return Response;
    }

} // namespace NDnnProto