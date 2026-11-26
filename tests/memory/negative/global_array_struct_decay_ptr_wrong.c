// expect: error check @ L19
// c-only: the program is global_array_struct_decay_ptr.c's, which is C only (in C++ the escape of the decay makes g unknown: the twin negative/global_array_struct_decay_ptr_escape.cpp)
// twin: tests/memory/global_array_struct_decay_ptr.c
// Pins: 0064 item 9 (audit round C2, finding 3, `struct P *pg = g;` in C): the same program with a claim about the value read through the pointer that is false, pg->x == 2 (the first element's x is 1): only that check fails, the check of the control (g[1].x == 3, pg == &g[0], pg[1].y == 4) holds, so pg is the pointer to the first element and the failure is the claim's
// Registered from audit-C2 probe a11c (measured on snapshot a044aff629b8: its second check, `pg->x == 2`, failed at its line and no other row); the rows' line is inferred from this layout
#include <cerastes.h>

struct P {
  int x;
  int y;
};

static struct P g[2] = {{1, 2}, {3, 4}};
struct P *pg = g;

int main(void)
{
  CERASTES_CHECK(g[1].x == 3 && pg == &g[0] && pg[1].y == 4);
  CERASTES_CHECK(pg->x == 2);  // <- here
  return g[1].x - 3;
}
