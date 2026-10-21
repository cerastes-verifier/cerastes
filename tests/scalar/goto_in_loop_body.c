// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0015 item 2 and 0014 item 3: a goto to a label at the end of the loop body acts as a continue
// The invariant is checked at the end of the body, where the paths of the goto and of the fall-through meet.
#include <cerastes.h>

int count_below(int n, int limit)
    CERASTES_REQUIRES(0 <= n && n <= 1000 && 0 <= limit && limit <= 1000)
    CERASTES_ENSURES(r, 0 <= r && r <= n)
{
  int count = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && 0 <= count && count <= i)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    if (i >= limit) {
      goto next;
    }
    count += 1;
next:
    i += 1;
  }
  return count;
}
