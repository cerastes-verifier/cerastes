// expect: error check @ L14
// mode: c23, c++20
// twin: tests/memory/addr_param.c
// Pins: 0037 item 4: the write through q changed a, so a == 2 fails
#include <cerastes.h>
#include <stddef.h>

int set_one(int a)
    CERASTES_REQUIRES(a < 100)
    CERASTES_ENSURES(r, r == 1)
{
  int *q = &a;
  *q = 1;
  CERASTES_CHECK(a == 2);
  return a;
}

int read_back(int a)
    CERASTES_ENSURES(r, r == a)
{
  int *q = &a;
  return *q;
}
