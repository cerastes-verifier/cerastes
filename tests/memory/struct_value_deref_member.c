// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0045 item 9 (flip of reject_struct_value_deref_member.c): *s.q read in a contract once an atom owns it
#include <cerastes.h>
#include <stddef.h>
struct P {
  int *q;
};

int f(struct P s)
    CERASTES_REQUIRES(cerastes_owns(*s.q) && *s.q == 1)
    CERASTES_ENSURES(r, cerastes_owns(*s.q) && r == 1)
{
  return *s.q;
}
