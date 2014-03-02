namespace Rcpp {
    template <> SEXP wrap<SRMLayer>( const SRMLayer& el ) {
            Rcpp::Language call( "new", Symbol( "SRMLayer" ), el) ;
            return call.eval();
    };

    template <> SRMLayer as<SRMLayer>( SEXP s )  { // throw(not_compatible) {
            try {
                if ( TYPEOF(s) != S4SXP ) {
                    ::Rf_error( "supplied object is not of type SRMLayer." );
                }

                Rcpp::S4 s4obj( s );
                if ( !s4obj.is("Rcpp_SRMLayer" ) ) {
                    ::Rf_error( "supplied object is not of type SRMLayer." );
                }

                Rcpp::Environment env( s4obj );
                Rcpp::XPtr<SRMLayer> xptr( env.get(".pointer") );

                    return SRMLayer(xptr->N, xptr->ids, xptr->W, xptr->id_conns, xptr->syn, xptr->syn_spec, xptr->a, xptr->C, xptr->pacc, xptr->incr);
            }
            catch(...) {
                ::Rf_error( "supplied object could not be converted to SRMLayer." );
            }
    };
    template <> SEXP wrap<FBLayer>( const FBLayer& el ) {
            Rcpp::Language call( "new", Symbol( "FBLayer" ), el) ;
            return call.eval();
    };

    template <> FBLayer as<FBLayer>( SEXP s )  { // throw(not_compatible) {
            try {
                if ( TYPEOF(s) != S4SXP ) {
                    ::Rf_error( "supplied object is not of type FBLayer." );
                }

                Rcpp::S4 s4obj( s );
                if ( !s4obj.is("Rcpp_FBLayer" ) ) {
                    ::Rf_error( "supplied object is not of type FBLayer." );
                }

                Rcpp::Environment env( s4obj );
                Rcpp::XPtr<FBLayer> xptr( env.get(".pointer") );

                    return FBLayer( *xptr );
            }
            catch(...) {
                ::Rf_error( "supplied object could not be converted to FBLayer." );
            }
    };

};


