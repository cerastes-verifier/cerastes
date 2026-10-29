// expect: rejected cerastes-unsupported_type @ L8
// mode: c++20
// twin: tests/memory/restrict_owned.c
// Pins: 0037 item 2: __restrict in C++ is an extension cerastes rejects (re-expected by M8)
#include <cerastes.h>
#include <stddef.h>

int get(int *__restrict p)
    CERASTES_REQUIRES(cerastes::owns(*p))
    CERASTES_ENSURES(r, cerastes::owns(*p))
{
  return *p;
}
