#pragma once

#include "http.h"

namespace NDnn {

    class TResponseBuilder {
    public:
        TResponseBuilder(const THttpRequest& req) {
            Response.Version = req.Version;
        }

        TResponseBuilder& StaticFile(TString file);

        void DeduceMimeTypeFromFile(TString path);

        TResponseBuilder& Good();

        TResponseBuilder& Created();

        TResponseBuilder& Accepted();

        TResponseBuilder& NotFound();

        TResponseBuilder& BadRequest();

        TResponseBuilder& InternalError();
        
        TResponseBuilder& ServiceUnavailable();

        TString& Body();

        TResponseBuilder& Body(const TString&& body);

        THttpResponse FormResponse();

    private:
        THttpResponse Response;
    };

} // namespace NDnn
