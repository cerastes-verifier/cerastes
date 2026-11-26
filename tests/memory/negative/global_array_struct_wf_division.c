// expect: error division-by-zero @ L20
// expect: error division-by-zero @ L27
// expect: error division-by-zero @ L34
// expect: error division-by-zero @ L42
// mode: c23, c++20
// twin: tests/memory/global_array_struct_wf.c
// Pins: 0064 items 9 and 11 (Part C2, audit round C1's finding 5 for a global array of structures; f__wf's ghosts, one per leaf path and slot, shared by no two): each function proves g[0].x != 0 in its requires and divides by a different value in its ensures, so the division is by zero in the ensures, once per function. other_leaf: g[0].y; other_slot: g[1].x; owned: under owns(g) the body may change g[0].x, so the ensures' value is not the requires' (the tie is made by a reads atom only); other_index: g[j].x against g[i].x
// Pre-registered from record 0064 items 9 and 11 and the audit's finding 5 (w01, w02, w03, w11's shapes over the global array), before any run: division-by-zero at the ensures line of each function
#include <cerastes.h>

struct P {
  int x;
  int y;
};

static struct P g[3] = {{1, 2}, {3, 4}};

int other_leaf(void)
    CERASTES_REQUIRES(cerastes_reads(g) && g[0].x != 0)
    CERASTES_ENSURES(r, cerastes_reads(g) && (r == 0 || r == 10 / g[0].y))
{
  return g[0].x * 0;
}

int other_slot(void)
    CERASTES_REQUIRES(cerastes_reads(g) && g[0].x != 0)
    CERASTES_ENSURES(r, cerastes_reads(g) && (r == 0 || r == 10 / g[1].x))
{
  return g[0].x * 0;
}

int owned(void)
    CERASTES_REQUIRES(cerastes_owns(g) && g[0].x != 0)
    CERASTES_ENSURES(r, cerastes_owns(g) && (r == 0 || r == 10 / g[0].x))
{
  g[0].x = 0;
  return 0;
}

int other_index(int i, int j)
    CERASTES_REQUIRES(cerastes_reads(g) && 0 <= i && i < 3 && 0 <= j && j < 3 && g[i].x != 0)
    CERASTES_ENSURES(r, cerastes_reads(g) && (r == 0 || r == 10 / g[j].x))
{
  (void)j;
  return g[i].x * 0;
}
