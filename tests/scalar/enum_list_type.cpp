// expect: verified
// expect: native-ok
// Pins: 0018 item 10 and 0022: inside the enumerator list C++ types U as unsigned int (the initializer 2u), so V = U * 1073741824 > 0 is computed in unsigned (2^31 fits) and is 1
// In C23 the same list gives U the type int, where 2 * 1073741824 overflows: reject_enum_list_type.c is rejected there.
#include <cerastes.h>

enum {
  U = 2u,
  V = U * 1073741824 > 0
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
