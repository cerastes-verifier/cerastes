// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0045 item 9 (flip of reject_struct_value_live_member.c): live(s.q) of a by-value parameter's pointer member: the callee compares it
#include <cerastes.h>
#include <stddef.h>
struct P {
  int *q;
};

int f(struct P s)
    CERASTES_REQUIRES(cerastes_live(s.q))
    CERASTES_ENSURES(r, cerastes_live(s.q) && r == (s.q == NULL))
{
  return s.q == NULL;
}
