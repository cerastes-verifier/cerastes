// expect: error null-dereference @ L14
// mode: c23, c++20
// twin: tests/memory/owning_typedef_twice.c
// Pins: 0040 item 2 (re-audit of audit round 2's B1): the owning parameter of the typedef'd type may be null: *p without a test applies * to null
#include <cerastes.h>
#include <stddef.h>

typedef void fn_t(CERASTES_OWNING int *p);
fn_t f;
fn_t f;

void f(CERASTES_OWNING int *p)
{
  *p = 1;
}
