#pragma once


#define CAST_TYPE(const_type, from_where) \
    const const_type *cast = dynamic_cast<const const_type*>(from_where);\
    if(!cast) {\
        cerr << "Instance was created with constants different type\n";\
        terminate();\
    }\


