// expect: error postcondition @ L17
// mode: c23, c++20
// twin: tests/memory/array_element_param.c
// Pins: 0059 item 4: the callee's contract says nothing about *p, so a[1] == 5 is not known
// Pre-registered from record 0059 item 4
#include <cerastes.h>

void set(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p));

int use_set(void)
    CERASTES_ENSURES(r, r == 5)
{
  int a[3] = {1, 2, 3};
  set(&a[1]);
  return a[1];
}
