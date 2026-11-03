// expect: error postcondition @ L21
// c-only: restrict and compound literal objects do not exist in C++
// twin: tests/memory/struct_literal_for_decl.c
// Pins: 0044 item 7: the loop leaves s == 3: the postcondition r == 2 fails at the return
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int in_for_decl(void)
    CERASTES_ENSURES(r, r == 2)
{
  int s = 0;
  CERASTES_INVARIANT(0 <= s && s <= 3)
  CERASTES_DECREASES(3 - s)
  for (int *p = &(int){1}; s < 3; s += 1) {
    *p = s;
  }
  return s;  // <- here
}
