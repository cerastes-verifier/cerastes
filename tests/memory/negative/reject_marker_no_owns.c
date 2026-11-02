// expect: rejected cerastes-unsupported_annotation @ L7
// mode: c23, c++20
// twin: tests/memory/init_must_init.c
// Pins: 0042 item 4 (question 88): a marker needs an owns(*p) atom in the requires (never lifted)
#include <cerastes.h>

void f(int *p CERASTES_MUST_INIT)  // <- here
{
  *p = 0;
}
