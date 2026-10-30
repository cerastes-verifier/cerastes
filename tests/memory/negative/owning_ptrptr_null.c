// expect: error null-dereference @ L10
// mode: c23, c++20
// twin: tests/memory/owning_ptrptr.c
// Pins: 0040 item 3: the owning T** parameter may be null
#include <cerastes.h>
#include <stddef.h>

void clear(CERASTES_OWNING int **pp)
{
  *pp = NULL;
}
