// expect: error check @ L18
// mode: c++20
// twin: tests/memory/global_array_struct_decay_ptr.c
// Pins: 0064 item 9 and 0041 item 3 (audit round C2, finding 3: the first escape direction of 0063 item 9 for an array of structures, in C++ where another unit's dynamic initializer may run before main): `pg`, an external pointer initialized with the decay of the internal array g, makes g unknown in main, so the check of the value the initializer wrote is not provable. The same program in C (the twin: its first check, with the pointer stored in the same place) verifies: the linkage rule differs by mode. The pointer-leaf direction is negative/global_array_struct_decay_ptr_escape_leaf.cpp
// Registered from audit-C2 probe a11 (measured on snapshot a044aff629b8 in C++: `check` at its check), before the first run on this build; the row's line is inferred from this layout
#include <cerastes.h>

struct P {
  int x;
  int y;
};

static struct P g[2] = {{1, 2}, {3, 4}};
struct P *pg = g;

int main()
{
  CERASTES_CHECK(g[1].x == 3);  // <- here
  return 0;
}
