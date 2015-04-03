AC_INIT([Rdnn], [1.0])

AC_PREFIX_PROGRAM(dnn)


#Set optional include path with option --with-dnn-include or
#environment variable DNN_INCLUDE

AC_ARG_WITH([dnn-include],
            AC_HELP_STRING([--with-dnn-include=INCLUDE_PATH],
                           [the location of DNN header files]),
            [dnn_include_path=$withval])
if test [ -n "$dnn_include_path" ] ; then
   DNN_INCLUDE=${dnn_include_path}
else
   if test [ -z "${DNN_INCLUDE}" ] ; then
      if test [ "$prefix" = "NONE" ]; then
         AC_MSG_ERROR("Location of DNN headers not defined. Use configure arg '--with-dnn-include' or environment variable 'DNN_INCLUDE'")
      else
         DNN_INCLUDE=${prefix}/include
      fi
   fi
fi

#AC_LANG_PUSH(C++)
#OCPPFLAGS=${CPPFLAGS}
#CPPFLAGS=-I${SNN_INCLUDE}
#AC_CHECK_HEADER(Console.h, [],
#             [AC_MSG_FAILURE("Problem with header file ${SNN_INCLUDE}/Console.h ")])
#CPPFLAGS=${OCPPFLAGS}
#AC_LANG_POP()


#Set optional library path with option --with-snn-lib or
#environment variable SNN_LIBS

AC_ARG_WITH([dnn-lib],
            AC_HELP_STRING([--with-dnn-lib=LIB_PATH],
                           [the location of the DNN library]),
            [dnn_lib_path=$withval])
if test [ -n "$dnn_lib_path" ] ; then
   DNN_LIB=$dnn_lib_path
else
   if test [ -z "${DNN_LIB}" ] ; then
      if test [ "${prefix}" = "NONE" ] ; then 
         AC_MSG_ERROR("Location of DNN library not defined. Use configure arg '--with-dnn-lib' or environment variable 'DNN_LIB'")
      else
         DNN_LIB=${prefix}/lib
      fi
   fi
fi

OLDFLAGS=${LDFLAGS}
AC_LANG_PUSH(C)
LDFLAGS=-L${DNN_LIB}
#AC_CHECK_LIB(snn, snn_version, [],
#             [AC_MSG_ERROR("Cannot load SNN library in ${SNN_LIB}")])

AC_LANG_POP()
LDFLAGS=${OLDFLAGS}

#Finally, set optional module directory using --with-snn-modules or
#environment variable SNN_MODULES

AC_SUBST(DNN_INCLUDE)
AC_SUBST(DNN_LIB)

AC_OUTPUT(src/Makevars)
