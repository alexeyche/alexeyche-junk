
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


static void
_finalizer(SEXP ext)
{
    if (NULL == R_ExternalPtrAddr(ext))
        return;
    Rprintf("finalizing\n");
    MessageCont *ptr = (MessageCont*) R_ExternalPtrAddr(ext);
    Free(ptr->cont);
    Free(ptr);
    R_ClearExternalPtr(ext);
}


SEXP r_run_server(SEXP port) {
    PROTECT(port = coerceVector(port, INTSXP));
    int port_i = asInteger(port);
    pthread_t t;
//    pthread_attr_t attr;
//    pthread_attr_init(&attr);
//    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    args_s args;
    MessageCont *mc = MessageContCreate();
    args.mc = mc;
    args.port = port_i;
    Rprintf("here\n");
    pthread_create( &t, NULL, get_message, (void*)&args);
    sleep(1);
//    pthread_attr_destroy(&attr);
   
    SEXP ext = PROTECT(R_MakeExternalPtr(mc, "message_contatiner", port));
    R_RegisterCFinalizerEx(ext, _finalizer, TRUE);
    UNPROTECT(2);
    return ext;
}

SEXP r_serv_inf(SEXP ext) {
    MessageCont *mc = (MessageCont*) R_ExternalPtrAddr(ext);
    Rprintf("num: %d\n", mc->num);
    Rprintf("num_read: %d\n", mc->num);
    return R_NilValue;
}

SEXP r_get_message(SEXP ext) {
   MessageCont *mc = (MessageCont*) R_ExternalPtrAddr(ext);
   if(mc->num_read == mc->num) {
        return R_NilValue;
   }
   int i;
   SEXP list, list_names;
   char *names[4] = {"x","nrow", "ncol", "name"};
   
   DoubleMessage *m = mc->cont[mc->num_read];
   mc->num_read+=1;
     
   SEXP x, nrow, ncol, name;
   
   PROTECT(x = allocVector(REALSXP, m->nrow*m->ncol)); 
   memcpy(REAL(x), m->x, sizeof(double)*m->nrow*m->ncol); 
   
   PROTECT(nrow = allocVector(INTSXP, 1));
   INTEGER(nrow)[0] = m->nrow;
   
   PROTECT(ncol = allocVector(INTSXP, 1));
   INTEGER(ncol)[0] = m->ncol;
   
   PROTECT(name = allocVector(STRSXP, 1));
   SET_STRING_ELT(name, 0, mkChar(m->name));
 
   // a character string vector of the "names" attribute of the objects in our list
   PROTECT(list_names = allocVector(STRSXP, 4));
   for(i = 0; i < 4; i++)
      SET_STRING_ELT(list_names, i,  mkChar(names[i]));

   PROTECT(list = allocVector(VECSXP, 4)); 
   SET_VECTOR_ELT(list, 0, x);
   SET_VECTOR_ELT(list, 1, nrow);
   SET_VECTOR_ELT(list, 2, ncol);
   SET_VECTOR_ELT(list, 3, name);
   setAttrib(list, R_NamesSymbol, list_names); //and attaching the vector names
   UNPROTECT(6);
   return list;
} 

