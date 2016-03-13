#include "http.h"

namespace NDnn {

	std::ostream& operator<< (std::ostream& stream, const THttpRequest& httpReq) {
        stream << httpReq.Method << " " << httpReq.RawPath << " " << httpReq.Version << "\r\n";
        for(const auto& h: httpReq.Headers) {
            stream << h.first << ": " << h.second << "\r\n";
        }
        stream << "\r\n";
        stream << httpReq.Body;
        return stream;
    }

    std::ostream& operator<< (std::ostream& stream, const THttpResponse& httpResp) {
        stream << httpResp.Version << " " << httpResp.Code << " " << httpResp.Status << "\r\n";
        for(const auto& h: httpResp.Headers) {
            stream << h.first << ": " << h.second << "\r\n";
        }
        stream << "\r\n";
        stream << httpResp.Body;
        return stream;
    }

    namespace {

        TString GetLine(TDeque<char>& bytes) {
            TString s;
            while (bytes.size() > 0) {
                char b = bytes.front();
                bytes.pop_front();
                s.push_back(b);
                
                if (b == '\n') {
                    return s;
                }
             }
             if (s.empty()) {
                throw TDnnException() << "Stream of bytes ended prematurely";   
             }
             return s;
        }

    } // namespace

    THttpRequest ParseHttpRequest(TDeque<char>&& bytes) {
        THttpRequest request;

        TString httpSpecString = GetLine(bytes);
        httpSpecString = NStr::Trim(httpSpecString);
        TVector<TString> httpSpec = NStr::Split(httpSpecString, ' ');

        ENSURE(httpSpec.size() >= 3, "Bad http specifications: " << httpSpecString);
        request.Method = httpSpec[0];
        request.RawPath = httpSpec[1];
        request.Version = httpSpec[2];

        TVector<TPair<TString, TString>> headers;
        while (true) {
            TString headerString = GetLine(bytes);
            
            if (NStr::Trim(headerString).empty()) {
                break;
            }

            TVector<TString> headersVals = NStr::Split(headerString, ':', 1);
            ENSURE(headersVals.size() == 2, "Failed to parse headers: " << headerString);

            request.Headers.push_back(MakePair(headersVals[0], headersVals[1]));
        }
        
        while (bytes.size()>0) {
            request.Body += GetLine(bytes);
        }
        
        TString path = NStr::LStrip(request.RawPath, "/");
        TVector<TString> pathSpl = NStr::Split(path, '?', 1);
        request.Path = NStr::Trim(pathSpl[0]);
        if (request.Path.empty()) {
            request.Path = "/";
        }
        if (pathSpl.size() == 2) {
            TVector<TString> queryVariables = NStr::Split(pathSpl[1], '&');
            for (const auto &var: queryVariables) {
                auto varValSpl = NStr::Split(var, '=', 1);
                ENSURE(varValSpl.size() == 2, "Illformed query string: " << var);
                request.UrlArgs.push_back(
                    MakePair(NStr::Trim(varValSpl[0]), NStr::Trim(varValSpl[1]))
                );

            }
            
        }
        
        return request;
    }

    TString GetDateAndTime() {
        time_t rawtime;
        struct tm * timeinfo;
        char buffer[80];

        time (&rawtime);
        timeinfo = localtime(&rawtime);

        strftime(buffer, 80, "%a, %d %b %Y %H:%M:%S",timeinfo);
        return TString(buffer);
    }

} // namespace NDnn