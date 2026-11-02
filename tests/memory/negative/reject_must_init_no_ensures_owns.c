// expect: rejected cerastes-unsupported_annotation @ L7
// mode: c23, c++20
// twin: tests/memory/init_must_init.c
// Pins: 0042 item 4 (question 88): must_init needs an owns(*p) atom in the ensures too (never lifted)
#include <cerastes.h>

void f(int *p CERASTES_MUST_INIT)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p))
{
  *p = 0;
}
