// expect: error null-dereference @ L13
// mode: c23, c++20
// twin: tests/memory/owning_typedef.c
// Pins: 0040 item 3 (audit N2): the typedef's owning parameter may be null, so *p without a test applies * to null
#include <cerastes.h>
#include <stddef.h>

typedef void fn_t(int *p CERASTES_OWNING);
fn_t f;

void f(int *p CERASTES_OWNING)
{
  *p = 1;
}
