AC_DEFUN([AX_LIB_BOOST],
[
has_boost=false
AC_ARG_WITH(boost,
    [  --with-boost=<path>     prefix of boost installation. e.g. /usr/local/include/boost-1_33_1], 
    [if test $withval == "no"
     then
       has_boost=false
     else
       has_boost=true
     fi],
    has_boost=false
)
BOOST_PREFIX=$with_boost
AC_SUBST(BOOST_PREFIX)

has_fieldmap=
AC_ARG_WITH(fieldmap,
    [  --with-fieldmap=<type>  select non-default field map implementation -
                          "boost::intrusive::sgtree", "boost::intrusive::rbtree",
                          or "boost::intrusive::avltree"], 
    [has_fieldmap=$withval],
    has_fieldmap="default"
)
has_slist=false
AC_ARG_WITH(slist-traversal,
    [  --with-slist-traversal  maintain a single linked list for faster tree traversal],
    [has_slist=$withval; test "$withval" = no || has_slist=true],
    has_slist=false
)
if test $has_boost = true
then
    BOOST_LIBS="-L${BOOST_PREFIX}/lib -lboost_system -lboost_thread"
    BOOST_CFLAGS="-I${BOOST_PREFIX}/include"
    AC_SUBST(BOOST_CFLAGS)
    AC_DEFINE(HAVE_BOOST, 1, Define if you have boost framework)
fi
])
AC_DEFUN([AX_LIB_BOOST_CHECK],
[
if test $has_boost = true
then
    AC_MSG_CHECKING(for Boost)
    AC_TRY_COMPILE(
                [#include <boost/version.hpp>],
                [int v = BOOST_VERSION;],
                AC_MSG_RESULT(yes),
                AC_MSG_ERROR(no))
fi
AC_MSG_CHECKING(for FieldMap container type)
case $has_fieldmap in
     boost::intrusive::sgtree)
       AC_DEFINE(ENABLE_BOOST_SGTREE, 1, Define for boost::intrusive::sgtree)
       if test $has_slist = true
       then
         AC_DEFINE(ENABLE_SLIST_TREE_TRAVERSAL, 1, Define for tree with list)
         AC_MSG_RESULT(boost::intrusive::sgtree with list)
       else
         AC_MSG_RESULT(boost::intrusive::sgtree)
       fi;;
     boost::intrusive::rbtree)
       AC_DEFINE(ENABLE_BOOST_RBTREE, 1, Define for boost::intrusive::rbtree)
       if test $has_slist = true
       then
         AC_DEFINE(ENABLE_SLIST_TREE_TRAVERSAL, 1, Define for tree with list)
         AC_MSG_RESULT(boost::intrusive::rbtree with list)
       else
         AC_MSG_RESULT(boost::intrusive::rbtree)
       fi;;
     boost::intrusive::avltree)
       AC_DEFINE(ENABLE_BOOST_AVLTREE, 1, Define for boost::intrusive::avltree)
       if test $has_slist = true
       then
         AC_DEFINE(ENABLE_SLIST_TREE_TRAVERSAL, 1, Define for tree with list)
         AC_MSG_RESULT(boost::intrusive::avltree with list)
       else
         AC_MSG_RESULT(boost::intrusive::avltree)
       fi;;
     *)
       AC_MSG_RESULT(defaulting to the Container::avlTree)
       ;;
esac
])
