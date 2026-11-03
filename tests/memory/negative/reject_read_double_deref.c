// expect: rejected cerastes-unsupported_annotation @ L9
// mode: c23, c++20
// twin: tests/memory/ptrptr_set.c
// Pins: 0040 item 6 (question 74), record 0043 item 15 (re-messaged by M4.5c1): a read of **pp in a contract is rejected until M4.5c2
#include <cerastes.h>
#include <stddef.h>

void f(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp) && **pp > 0)
{
  (void)pp;
}
