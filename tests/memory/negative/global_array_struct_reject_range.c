// expect: rejected cerastes-unsupported_annotation @ L16
// mode: c23, c++20
// twin: tests/memory/global_array_struct_calls.c
// Pins: 0064 item 1 (M4.5e2): owns_range(g, n) with g a global array of structures: a range's base is a pointer parameter, and a range on part of a global array is M4.5e2's
// Pre-registered from record 0064 item 1 (Part C2), before any run: rejected (the tag by negative/global_array_reject_expressions.c's owns_range row; to confirm at the joined run)
#include <cerastes.h>

struct P {
  int x;
  int y;
};

static struct P g[3] = {{1, 2}, {3, 4}};

int f(int n)
    CERASTES_REQUIRES(cerastes_owns_range(g, n))  // <- here
{
  return n + g[0].x;
}
