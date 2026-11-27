// expect: error postcondition @ L26
// mode: c23, c++20
// twin: tests/memory/struct_array_aggregate_ok.c
// Pins: 0064 item 15's last clause and question 348: the callee claims p->v[1].x == 8 and the body writes 9: postcondition at the closing brace (the positive keeps its Part C2 outcome; this is its error twin)
// Pre-registered from record 0064 item 15, before any run: postcondition at the closing brace of set (a Part C2 shape: expected to hold on the frozen build as well)
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct P {
  int x;
  int y;
};

struct T {
  struct P v[2];
  int n;
};

void set(struct T *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && p->v[1].x == 8 && p->n == 1)
{
  p->v[1].x = 9;
  p->n = 1;
}  // <- here
