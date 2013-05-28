
#include "R.h"
#include "Rinternals.h"

#include "sim_server.c"

SEXP listen_port(SEXP port) {
    PROTECT(port = coerceVector(port, INTSXP));
    int port_i = asInteger(port);
    Rprintf("%i\n", port_i);
    SEXP ab;
    PROTECT(ab = allocVector(REALSXP, 10)); 
    UNPROTECT(2);
    return(ab);
}
