// expect: error unsequenced @ L35
// expect: error unsequenced @ L42
// expect: error unsequenced @ L49
// mode: c23, c++20
// twin: tests/memory/global_array_struct_seq.c
// Pins: 0064 items 9 and 11 (Part C2, audit round C1's finding 1 for a global array of structures; the rows are the sequencing rows of a call that owns g, with no other check behind them): one function per row, each unsequenced at its expression, once. beside_read: wr() owns g and g[1].x is read in the same expression; beside_value: the same beside a by-value use of the element, take(g[1]); two_calls: wr() twice (both own g)
// Pre-registered from record 0064 items 9 and 11 and the audit's finding 1 (the shapes of its batch s over a parameter's range, here over a global array), before any run: one unsequenced per function at the line of its expression
#include <cerastes.h>

struct P {
  int x;
  int y;
};

static struct P g[3] = {{1, 2}, {3, 4}};

int wr(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(r, cerastes_owns(g) && g[1].x == 7 && r == 0)
{
  g[1].x = 7;
  return 0;
}

int take(struct P t)
    CERASTES_ENSURES(r, r == t.x)
{
  return t.x;
}

int beside_read(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(r, cerastes_owns(g))
{
  return wr() + g[1].x;
}

int beside_value(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(r, cerastes_owns(g))
{
  return wr() + take(g[1]);
}

int two_calls(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(r, cerastes_owns(g))
{
  return wr() + wr();
}
