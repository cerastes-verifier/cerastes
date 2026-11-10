// expect: rejected cerastes-unsupported_annotation @ L8
// mode: c23, c++20
// twin: tests/memory/not_invalidating_int_pointer.c
// Pins: 0058 item 2 (as revised): an atom on the marked parameter of a function declared not to return: it has no ensures to hold the promise (as 0042 item 4 says of must_init; never lifted)
// Pre-registered from record 0058 item 2 (as revised): rejected unsupported_annotation at the parameter
#include <cerastes.h>

[[noreturn]] void bail(int *p CERASTES_NOT_INVALIDATING)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p));

void use_bail(void)
{
  int x = 0;
  bail(&x);
}
