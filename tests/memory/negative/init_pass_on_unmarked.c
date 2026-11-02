// expect: error uninitialized @ L15
// mode: c23, c++20
// twin: tests/memory/init_pass_on.c
// Pins: 0042 item 7: the must_init parameter's pointee has no value on entry, and set needs one: uninitialized at the argument
#include <cerastes.h>

void set(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && *p == 1);

void init_bad(int *p CERASTES_MUST_INIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  set(p);  // <- here
}
