// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 item 4 and 10 (question 160 correction of array_ptr_loop_reassigned_false_alarm.c): pk[0] through a pointer local to a one-slot cell (int *pk = &k) in a loop, the shape of owning_field_loop_goto_inside.c: the loop place acc(pk.v_int) needs no offset_of < len, the bound row holds from what pk was given, so pk[0] verifies as *pk does
// Pre-registered from the probe of the audit's shape (verifies in both modes; no shape was found where *pk verifies and pk[0] fails)
#include <cerastes.h>

int f(int n)
    CERASTES_REQUIRES(0 <= n && n <= 3)
    CERASTES_ENSURES(r, r == 0)
{
  int k = 0;
  int *pk = &k;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    if (i == n) {
      goto skip;
    }
    pk[0] = i;
  skip:
    i = i + 1;
  }
  return 0;
}
