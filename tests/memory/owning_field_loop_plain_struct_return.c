// expect: verified
// expect: native-ok
// mode: c23, c++20
// Covers: 0056 item 11 (question 214, audit re-audit r4): a plain struct local (no owning leaf) and a loop with a `return` in it that stores through a pointer: the struct is not framed out of the loop (framing is for a struct with an owning leaf whose address does not escape and that the loop does not name, in a loop without a return or goto out), so the member read after the loop keeps its value
// Pre-registered from the re-audit's r4_plain_struct.c and edg-backend report 5 (verified, c23 and c++20, 2 s): verifies in both modes
#include <cerastes.h>

struct pt {
  int x;
  int y;
};

int f(int n)
    CERASTES_REQUIRES(0 <= n && n <= 3)
{
  struct pt s = { 1, 2 };
  int k = 0;
  int *pk = &k;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    *pk = i;
    if (i == n) {
      return 0;
    }
    i = i + 1;
  }
  return s.x;
}
