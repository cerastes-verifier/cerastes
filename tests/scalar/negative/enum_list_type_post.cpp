// expect: error postcondition @ L14
// twin: tests/scalar/enum_list_type.cpp
// Pins: 0018 item 10: in C++ V = U - 3 > 0 is an unsigned comparison and is 1, not 0
#include <cerastes.h>

enum {
  U = 2u,
  V = U - 3 > 0
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
