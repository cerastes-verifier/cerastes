// expect: error null-dereference @ L10
// mode: c23, c++20
// twin: tests/memory/owning_param.c
// Pins: 0040 item 3: an owning parameter may be null, so *p without a test applies * to null
#include <cerastes.h>
#include <stddef.h>

void set_one(CERASTES_OWNING int *p)
{
  *p = 1;
}
