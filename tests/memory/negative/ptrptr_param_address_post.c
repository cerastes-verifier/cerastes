// expect: error postcondition @ L14
// mode: c23, c++20
// twin: tests/memory/ptrptr_param_address.c
// Pins: 0040 item 2: the result is old(*p) + 1, not + 2
#include <cerastes.h>
#include <stddef.h>

int via_param(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && *p < 100)
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == cerastes_old(*p) + 2)
{
  int **pp = &p;
  **pp = **pp + 1;
  return **pp;
}
