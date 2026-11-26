// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 7 and 10 (loops over a parameter's range of structures; the loop carries p's range per leaf path): scan reads p[i].x and keeps p[0].x in its invariant (a read of the range framed by the loop's place); fill_y writes the leaf y of every element and keeps p[0].x == first (a store to the long member y reaches the places of that field only, so it does not reach the int member x that the invariant keeps: y is a long for that reason, record item 10's reach rule; a written invariant on another int member is kept by a loop that stores an int member, the audit's l14; the automatic conjuncts alone keep no other int leaf, the listed false alarm pinned in negative/range_struct_loop_keep_false_alarm.c); keeper sets p[0].x, calls scribble (which writes x through the range and states it) and keeps the value it states through a loop that writes y only. The program of the staging pin array_struct_reject_loop_pointer.c holds a structure atom only and stays a pin
// Pre-registered from record 0064 items 7, 8 and 10 (Part C1), before any run: verifies [CORRECTED once under question 160 (first run, j2a): in c++20 EDG warns #550 (set but never used) on `first`, which only the invariant reads, and the harness runs warning-free; fill_y now stores it (`p[0].y = first;`), replacing the `(void)first;` that EDG's C++ does not count as a use (c23 was silent)]
#include <cerastes.h>
#include <stddef.h>

struct P {
  int x;
  long y;
};

int scan(struct P *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && 0 < n)
    CERASTES_ENSURES(r, cerastes_owns_range(p, n) && 0 <= r && r <= n)
{
  int first = p[0].x;
  int c = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && 0 <= c && c <= i && p[0].x == first)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    if (p[i].x > first) {
      c = c + 1;
    }
    i = i + 1;
  }
  return c;
}

void fill_y(struct P *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && 0 < n)
    CERASTES_ENSURES(cerastes_owns_range(p, n) && p[0].x == cerastes_old(p[0].x))
{
  int first = p[0].x;
  p[0].y = first;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && p[0].x == first)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    p[i].y = 0;
    i = i + 1;
  }
}

void scribble(struct P *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && 0 < n)
    CERASTES_ENSURES(cerastes_owns_range(p, n) && p[0].x == 7)
{
  (void)n;
  p[0].x = 7;
}

void keeper(struct P *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && 0 < n)
    CERASTES_ENSURES(cerastes_owns_range(p, n))
{
  p[0].x = 7;
  scribble(p, n);
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && p[0].x == 7)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    p[i].y = 0;
    i = i + 1;
  }
}
