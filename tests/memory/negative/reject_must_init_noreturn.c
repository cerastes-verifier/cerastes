// expect: rejected cerastes-unsupported_annotation @ L7
// mode: c23, c++20
// twin: tests/memory/init_noreturn.c
// Pins: 0042 item 4: must_init on a function declared not to return: it has no ensures to hold the promise (never lifted)
#include <cerastes.h>

[[noreturn]] void bail(int *p CERASTES_MUST_INIT)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p));

void use_bail(void)
{
  int x;
  bail(&x);
}
