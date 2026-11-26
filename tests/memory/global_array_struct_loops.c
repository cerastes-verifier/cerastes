// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 10 (Part C2): in main a global array of structures is, per leaf, an automatic place of a loop: a read-only sum loop keeps the values (s <= 3 * i holds from the initial list) and a fill loop of g[i].x keeps the permission, the flags and the range, not the values of that leaf (the values of the long member y stay: the store to the int member x reaches the places of the int field only, and an int y would be item 10's listed false alarm, the same-type member the automatic conjunct does not keep)
// Pre-registered from record 0064 item 10 (Part C2), before any run: verifies [CORRECTED once under question 160 (first run, j3a-t1): `int y` drew check @ L33 (the listed false alarm of item 10: a loop that writes g[i].x cannot keep the other int member by the automatic conjunct); y is now a long, as range_struct_loop.c's struct P, and the check stays]
#include <cerastes.h>

struct P {
  int x;
  long y;
};

static struct P g[3] = {{1, 2}, {3, 4}};

int main(void)
{
  int s = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3 && 0 <= s && s <= 3 * i)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    s += g[i].x;
    i += 1;
  }
  CERASTES_CHECK(s <= 9);
  i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    g[i].x = i;
    i += 1;
  }
  CERASTES_CHECK(g[0].y == 2 && g[1].y == 4);
  return s - s;
}
