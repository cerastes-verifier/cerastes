// expect: error null-dereference @ L11
// mode: c23, c++20
// twin: tests/memory/owning_merged.c
// Pins: 0040 item 3 (question 75): beside p == NULL || owns(*p) the guard stays, so *p without a test applies * to null
#include <cerastes.h>
#include <stddef.h>

void set_maybe(CERASTES_OWNING int *p)
    CERASTES_REQUIRES(p == NULL || cerastes_owns(*p))
{
  *p = 2;
}
