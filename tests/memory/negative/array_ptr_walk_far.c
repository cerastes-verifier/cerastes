// expect: error permission @ L20
// mode: c23, c++20
// twin: tests/memory/array_ptr_walk.c
// Pins: 0059 items 4 and 10 (question 229): (the postcondition is widened to r <= 12 so that the exit state with i == 4 is not a second error) one iteration too many forms &a[3] (allowed, one past the end) and reads it: permission, since no permission exists at len
// Pre-registered from record 0059 items 4 and 10 (Part D), before the code
#include <cerastes.h>

int sum(void)
    CERASTES_ENSURES(r, 0 <= r && r <= 12)
{
  int a[3] = {1, 2, 3};
  int i = 0;
  int s = 0;
  CERASTES_INVARIANT(0 <= i && i <= 4)
  CERASTES_INVARIANT(0 <= s)
  CERASTES_INVARIANT(s <= 3 * i)
  CERASTES_DECREASES(4 - i)
  while (i <= 3) {
    int *p = &a[i];
    s = s + *p;
    i = i + 1;
  }
  return s;
}
