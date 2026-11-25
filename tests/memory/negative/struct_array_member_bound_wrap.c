// expect: error overflow @ L9
// mode: c23, c++20
// twin: tests/memory/struct_array_member_declared.c
// Pins: 0063 items 2 and 14 (N9): the bound of an array member is checked as written wherever the structure is accepted: 65536u * 65536u wraps (the front end says unsigned char[1]), overflow at the bound
// Pre-registered from record 0063 item 2 (Part C), before any run: the one error
#include <cerastes.h>

struct W {
  unsigned char buf[65536u * 65536u + 1];  // <- here
  int n;
};

int use(void)
{
  struct W w;
  w.n = 1;
  return w.n;
}
