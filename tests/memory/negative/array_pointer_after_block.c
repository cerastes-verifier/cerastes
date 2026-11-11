// expect: error permission @ L15
// mode: c23, c++20
// twin: tests/memory/array_element_param.c
// Pins: 0059 item 6 and 0032 item 6 (audit round 2, item 5d): q = &a[1] stored out of the block, the block ended: the read through q has no permission
// Pre-registered from audit round 2 of record 0059 (item 5d)
#include <cerastes.h>

int f(void)
{
  int *q;
  {
    int a[2] = {1, 2};
    q = &a[1];
  }
  return *q;
}
