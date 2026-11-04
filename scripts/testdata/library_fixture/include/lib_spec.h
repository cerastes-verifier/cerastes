// Library fixture: a predicate and a pure function in a library header are accepted (a system header's would be
// spec_marker_in_system_file).  Their bodies are not encoded before M4.5f.
#ifndef LIB_SPEC_H
#define LIB_SPEC_H
#include <cerastes.h>

CERASTES_PREDICATE bool lib_nonneg(int x)
{
  return x >= 0;
}

CERASTES_PURE int lib_twice(int x)
{
  return x + x;
}

#endif
