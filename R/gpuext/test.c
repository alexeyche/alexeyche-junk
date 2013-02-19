#include <R.h>
#include <Rdefines.h>

struct cudamat {
    float* data_host;
    float* data_device;
    int on_device;
    int on_host;
    int size[2];
    int is_trans; // 0 or 1
    int owns_data;
};

const int N_MAX=15;

static void
_finalizer(SEXP ext)
{
    if (NULL == R_ExternalPtrAddr(ext))
        return;
    Rprintf("finalizing\n");
    char *ptr = (char *) R_ExternalPtrAddr(ext);
    Free(ptr);
    R_ClearExternalPtr(ext);
}

SEXP
create(SEXP info)
{
    char *x = Calloc(N_MAX, char);
    snprintf(x, N_MAX, "my name is joe");
    SEXP ext = PROTECT(R_MakeExternalPtr(x, "blah blah", info));
    R_RegisterCFinalizerEx(ext, _finalizer, TRUE);
    UNPROTECT(1);

    return ext;
}

SEXP
get(SEXP ext)
{
    return mkString((char *) R_ExternalPtrAddr(ext));
}

SEXP
set(SEXP ext, SEXP str)
{
    char *x = (char *) R_ExternalPtrAddr(ext);
    snprintf(x, N_MAX, CHAR(STRING_ELT(str, 0)));
    return ScalarLogical(TRUE);
}

SEXP setList(SEXP info) {
   int i;
   SEXP list, list_names;
   char *names[1] = {"ref"};
   
   char *x = Calloc(N_MAX, char);
   snprintf(x, N_MAX, "my name is joe");
   SEXP ext = PROTECT(R_MakeExternalPtr(x, R_NilValue, info));
   R_RegisterCFinalizerEx(ext, _finalizer, TRUE);


   // a character string vector of the "names" attribute of the objects in our list
   PROTECT(list_names = allocVector(STRSXP, 1));
   for(i = 0; i < 1; i++)
      SET_STRING_ELT(list_names, i,  mkChar(names[i]));
 
   PROTECT(list = allocVector(VECSXP, 1)); // Creating a list with 2 vector elements
   SET_VECTOR_ELT(list, 0, ext);
   setAttrib(list, R_NamesSymbol, list_names); //and attaching the vector names
   UNPROTECT(3);
   return list;
}   
