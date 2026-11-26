// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 6 and 10 (audit round C2, finding 3, the block of a structure local with an array member of structures): a pointer into the member array (the address of a leaf, the decay of the array, the address of an element) is read while the block of the structure is alive. The twin negative/struct_member_array_struct_scope_ended.c reads each pointer after the block has ended
// Registered from audit-C2 probe g05 (use, use2, use3: measured on snapshot a044aff629b8 with the read after the block, `permission`, `lifetime`, `permission`), with the read moved inside the block and stored in w before the block ends; unmeasured (g05's three functions are the twin's, this file is their control)
#include <cerastes.h>

struct P {
  int x;
  int y;
};

struct T {
  struct P v[2];
  int n;
};

int use(void)
    CERASTES_ENSURES(r, r == 3)
{
  int *q;
  int w;
  {
    struct T t = {{{1, 2}, {3, 4}}, 5};
    q = &t.v[1].x;
    w = *q;
  }
  return w;
}

int use2(void)
    CERASTES_ENSURES(r, r == 3)
{
  struct P *q;
  int w;
  {
    struct T t = {{{1, 2}, {3, 4}}, 5};
    q = t.v;
    w = q[1].x;
  }
  return w;
}

int use3(void)
    CERASTES_ENSURES(r, r == 4)
{
  struct P *q;
  int w;
  {
    struct T t = {{{1, 2}, {3, 4}}, 5};
    q = &t.v[1];
    w = q->y;
  }
  return w;
}
