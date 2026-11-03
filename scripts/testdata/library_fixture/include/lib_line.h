// Library fixture: a #line directive in a library file is rejected, as in a user header (decision 0011 item 4).
#ifndef LIB_LINE_H
#define LIB_LINE_H
#include <cerastes.h>

static inline int lib_renumbered(int x)
{
#line 200
  return x;
}

#endif
