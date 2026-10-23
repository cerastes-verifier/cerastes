// expect: error postcondition @ L14
// twin: tests/scalar/enum_list_type.cpp
// Pins: 0018 item 10 and 0022: in C++ V = U * 1073741824 > 0 is computed in unsigned and is 1, not 0
#include <cerastes.h>

enum {
  U = 2u,
  V = U * 1073741824 > 0
};

int v_value(void)
    CERASTES_ENSURES(r, r == 0)
{
  return V;
}

int u_value(void)
    CERASTES_ENSURES(r, r == 2)
{
  return U;
}
