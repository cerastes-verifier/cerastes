// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0039 item 4: int s = (x = (bump(&x), 2)): the write inside the expression follows the value computation of its operands, so it is emitted after the call (0025 item 2's placement would write first and leave x == 3); the statement form x = (bump(&x), 2) is unchanged (P60 C1)
#include <cerastes.h>
#include <stddef.h>

int bump(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && *p < 100)
    CERASTES_ENSURES(r, cerastes_owns(*p) && *p == cerastes_old(*p) + 1 && r == *p)
{
  *p += 1;
  return *p;
}

int comma_write(void)
    CERASTES_ENSURES(r, r == 22)
{
  int x = 1;
  int s = (x = (bump(&x), 2));
  return s * 10 + x;
}
