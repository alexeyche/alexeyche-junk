#pragma once

#include <dnn/base/base.h>
#include <dnn/util/log/log.h>
#include <dnn/util/string.h>
#include <dnn/util/optional.h>

#include <ctime>
#include <fstream>

namespace NDnn {

    struct THttpRequest {
        TString Method;
        TString RawPath;
        TString Version;

        TString Path;
        std::map<TString, TString> KeywordsMap;

        TVector<TPair<TString, TString>> UrlArgs;

        TVector<TPair<TString, TString>> Headers;

        TString Body;
    };

    struct THttpResponse {
        TString Version;
        ui32 Code;
        TString Status;

        TVector<TPair<TString, TString>> Headers;

        TString Body;
    };

    template <typename T>
    T FindUrlArg(const THttpRequest& req, const TString& varName, TOptional<T> def = TOptional<T>()) {
        auto varPtr = std::find_if(
            req.UrlArgs.begin(), 
            req.UrlArgs.end(), 
            [&](const TPair<TString, TString>& arg) { 
                return arg.first == varName; 
            }
        );
        if (varPtr == req.UrlArgs.end()) {
            if (def) {
                return *def;
            }
            throw TDnnElementNotFound() << "`variable_name' is not found in url arguments";
        }
        return NStr::As<T>(varPtr->second);
    }

    std::ostream& operator<< (std::ostream& stream, const THttpRequest& httpReq);

    std::ostream& operator<< (std::ostream& stream, const THttpResponse& httpResp);

    THttpRequest ParseHttpRequest(TDeque<char>&& bytes);

    TString GetDateAndTime();    

} // namespace NDnn
