// expect: error overflow @ L8
// twin: tests/scalar/enum_list_type.cpp
// Pins: 0018 item 10 and 0022: in C++ U is unsigned int inside the list, so U - 3 is 2u - 3u, an overflow at the enumerator (exit 1)
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
