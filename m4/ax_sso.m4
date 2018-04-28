AC_DEFUN([AX_SSO],
[
has_sso=false
AC_ARG_WITH(sso,
    [  --with-sso              short string optimization], 
    [if test $withval == "no"
     then
       has_sso=false
     else
       has_sso=true
     fi],
    has_sso=false
)

])
AC_DEFUN([AX_SSO_CHECK],
[
if test $has_sso = true
then
    sso_level=2
    sso_bytes=24
    if test "x$with_sso" == "x1"
    then
      sso_level=1
      sso_bytes=16
    fi 
    AC_MSG_CHECKING(for short string optimization)
    AC_TRY_COMPILE(
                [#include <string>
                 #include <stdint.h>
                 template <bool> struct s{};
                 template <> struct s<true> { typedef int type; };],
                [s<(sizeof(std::string) < 2 * sizeof(intptr_t))>::type v;],
                AC_MSG_RESULT($sso_bytes bytes),
                AC_MSG_ERROR(no))
   AC_DEFINE_UNQUOTED(ENABLE_SSO, ${sso_level}, Define to set short string optimization width)
fi
])
