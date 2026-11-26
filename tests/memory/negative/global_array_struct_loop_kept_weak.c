// expect: error check @ L24
// mode: c23, c++20
// twin: tests/memory/global_array_struct_loops.c
// Pins: 0064 item 10: after a loop that writes g[i].x the values of that leaf are not kept by the automatic conjunct (only the permission, the flags and the range are), so g[0].x == 1 is not a fact
// Pre-registered from record 0064 item 10 (Part C2), before any run: the one error [CORRECTED once under question 160 (first run, j3a-t1): EDG warned #550 for the static g only the check read; the return reads g[1].y * 0 (the row L24 held at its line, check)]
#include <cerastes.h>

struct P {
  int x;
  int y;
};

static struct P g[3] = {{1, 2}, {3, 4}};

int main(void)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    g[i].x = i;
    i += 1;
  }
  CERASTES_CHECK(g[0].x == 1);  // <- here
  return i - 3 + g[1].y * 0;
}
