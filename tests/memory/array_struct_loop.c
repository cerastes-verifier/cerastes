// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 10 (Part B; S48 (d)): loops over a local array of structures, each leaf its own array place, with the reach rule of the item: a store to a member of an element of a local array reaches every place of that member's field (every int leaf of every element when an int member is written), so the automatic values-kept conjunct is lost for the other int members. sum writes no memory cell (its accumulator and index are plain locals, not address-taken), reads v[i].x, and relies on the values of x kept (each x is at most 5); fill writes the int member v[i].x and afterwards claims only the long member y, which an int write does not reach, so the checks after the loop on v[1].y and v[2].y hold. The loop invariants of sum and fill read no memory; member_inv (record item 10 as the lowering builds it: `v[e].x` of a NAMED LOCAL array is a loop invariant read the loop's places frame) reads v[1].x in an invariant of a loop that writes no memory cell, so the value kept at entry is the invariant's. No check reads an int leaf after a loop that writes an int cell
// Pre-registered from record 0064 item 10 (Part B), before any run (revised before any run to the item's reach rule: leaf-precise reach is not built): verifies
#include <cerastes.h>

struct P {
  int x;
  int y;
};

struct Q {
  int x;
  long y;
};

int sum(void)
    CERASTES_ENSURES(r, 0 <= r && r <= 15)
{
  struct P v[3] = {{1, 2}, {3, 4}, {5, 6}};
  int s = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_INVARIANT(0 <= s)
  CERASTES_INVARIANT(s <= 5 * i)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    s = s + v[i].x;
    i = i + 1;
  }
  return s;
}

int fill(int n)
    CERASTES_REQUIRES(0 <= n && n <= 3)
    CERASTES_ENSURES(r, r == n)
{
  struct Q v[3] = {{0, 2}, {0, 4}, {0, 6}};
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    v[i].x = i;
    i = i + 1;
  }
  CERASTES_CHECK(v[1].y == 4 && v[2].y == 6);
  return i + (int)(v[1].y * 0);
}

int member_inv(void)
    CERASTES_ENSURES(r, 0 <= r && r <= 15)
{
  struct P v[3] = {{1, 2}, {3, 4}, {5, 6}};
  int s = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_INVARIANT(0 <= s)
  CERASTES_INVARIANT(s <= 5 * i)
  CERASTES_INVARIANT(v[1].x == 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    s = s + v[i].x;
    i = i + 1;
  }
  return s;
}
