// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0063 items 1 and 3 (flip A of M4.5e1e1, was negative/reject_struct_array_member.c, rejected by 0043 item 2 until now): a structure with an array member is accepted, even when the program never uses it; the program is unchanged, line for line (twin negative/struct_array_member_bound_wrap.c) (Pre-registered from record 0063 item 2 (Part C), before any run: verifies)
#include <cerastes.h>

struct A {
  int a[2];  // (was the rejected line)
  int b;
};

int ident(int a)
{
  return a;
}
