// expect: rejected cerastes-unsupported_annotation @ L9
// mode: c23, c++20
// twin: tests/memory/init_ref_to_uninit.c
// Pins: 0042 item 6 (question 86): *p in the ensures of a ref_to_uninit-only parameter: its flag is not promised on return (never lifted); must_init's ensures may read it
#include <cerastes.h>

int f(int *p CERASTES_REF_TO_UNINIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p) && *p == r)  // <- here
{
  *p = 1;
  return 1;
}
