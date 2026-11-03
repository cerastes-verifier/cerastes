// Library fixture: `#pragma GCC system_header` in a library file is rejected (decision 0046 item 3): a library
// file must not make itself a system file.
#ifndef LIB_PRAGMA_H
#define LIB_PRAGMA_H
#include <cerastes.h>
#pragma GCC system_header

static inline int lib_after_pragma(int x)
{
  return x;
}

#endif
