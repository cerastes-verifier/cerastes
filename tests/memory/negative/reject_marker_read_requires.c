// expect: rejected cerastes-unsupported_annotation @ L9
// mode: c23, c++20
// twin: tests/memory/init_must_init.c
// Pins: 0042 item 6 (question 86): *p in the requires of a marked parameter: its flag is not promised on entry (never lifted)
#include <cerastes.h>

void f(int *p CERASTES_MUST_INIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_REQUIRES(*p > 0)  // <- here
    CERASTES_ENSURES(cerastes_owns(*p))
{
  *p = 0;
}
