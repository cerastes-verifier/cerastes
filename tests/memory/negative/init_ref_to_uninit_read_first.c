// expect: error uninitialized @ L11
// mode: c23, c++20
// twin: tests/memory/init_ref_to_uninit.c
// Pins: 0042 item 5: a ref_to_uninit parameter's *p may have no value on entry, so reading it before a write is reported (0019)
#include <cerastes.h>

int peek(int *p CERASTES_REF_TO_UNINIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  return *p;  // <- here
}
