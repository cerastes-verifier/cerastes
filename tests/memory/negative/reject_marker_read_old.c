// expect: rejected cerastes-unsupported_annotation @ L9
// mode: c23, c++20
// twin: tests/memory/init_must_init.c
// Pins: 0042 item 6 (question 86): old(*p) of a marked parameter reads the pointee on entry (never lifted)
#include <cerastes.h>

void f(int *p CERASTES_MUST_INIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && *p == cerastes_old(*p))  // <- here
{
  *p = 0;
}
