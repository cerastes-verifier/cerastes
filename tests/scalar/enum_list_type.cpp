// expect: verified
// expect: native-ok
// Pins: 0018 item 10: inside the enumerator list C++ types U as unsigned int (the initializer 2u), so V = U - 3 > 0 is an unsigned comparison and is 1
// In C23 the same list gives U the type int and V == 0: reject_enum_list_type.c is rejected there.
#include <cerastes.h>

enum {
  U = 2u,
  V = U - 3 > 0
};

int v_value(void)
    CERASTES_ENSURES(r, r == 1)
{
  return V;
}

int u_value(void)
    CERASTES_ENSURES(r, r == 2)
{
  return U;
}
