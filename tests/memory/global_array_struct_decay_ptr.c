// expect: verified
// expect: native-ok
// c-only: in C++ an external pointer initialized with the decay of an internal array makes the array unknown in main (the twin negative/global_array_struct_decay_ptr_escape.cpp, decision 0041 item 3): this test is about C
// Pins: 0064 item 9 and 0041 item 3 (audit round C2, finding 3, `struct P *pg = g;`): in C an array of structures with internal linkage whose decay is the initializer of an external pointer stays known in main: nothing runs before main that could write through pg, and pg is the pointer loc(GB_g(), 0) of the first element (pg == &g[0], pg[1].y reads g[1].y). Single mode: in C++ the same program makes g unknown (another unit's dynamic initializer may write through pg), the twin negative/global_array_struct_decay_ptr_escape.cpp
// Registered from audit-C2 probe a11c (measured on snapshot a044aff629b8: its first check, `g[1].x == 3 && pg == &g[0] && pg[1].y == 4`, verified in C; the file keeps it and drops the second, false check `pg->x == 2`, which failed at its own line); the return statement is added so that g is read in code (-Wunused-variable), unmeasured
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
  return g[1].x - 3;
}
