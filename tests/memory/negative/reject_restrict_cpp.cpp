// expect: rejected cerastes-unsupported_type @ L8
// mode: c++20
// twin: tests/memory/restrict_dropped.c
// Pins: 0037 item 2: __restrict in C++ is an extension cerastes rejects (re-expected by M8) [0064 item 19: unchanged: restrict on a local, a global, a member, a pointee, a result or C++'s __restrict stays unsupported_type; only a parameter's restrict is item 19's rejection]
#include <cerastes.h>
#include <stddef.h>

int get(int *__restrict p)
    CERASTES_REQUIRES(cerastes::owns(*p))
    CERASTES_ENSURES(r, cerastes::owns(*p))
{
  return *p;
}
