// expect: error check @ L23
// mode: c++20
// twin: tests/memory/global_array_struct_decay_ptr.c
// Pins: 0064 item 9 and 0041 item 3 (audit round C2, finding 3: the second escape direction of 0063 item 9 for an array of structures): `tab`, an external array of structures whose second element holds the decay of the internal array g in its pointer leaf, makes g unknown in main (another unit's dynamic initializer may write through tab[1].q before main), so the check of the value the initializer wrote is not provable. Its control for the pointer-leaf direction with an internal holder is global_array_struct_escape.cpp, whose twin is negative/global_array_struct_escape_tab.cpp
// Registered from audit-C2 probe a12 (measured on snapshot a044aff629b8 in C++: `check` at its check), before the first run on this build; the row's line is inferred from this layout
#include <cerastes.h>

struct P {
  int x;
  int y;
};

struct H {
  struct P *q;
  int k;
};

static struct P g[2] = {{1, 2}, {3, 4}};
H tab[2] = {{0, 0}, {g, 0}};

int main()
{
  CERASTES_CHECK(g[1].x == 3);  // <- here
  return 0;
}
