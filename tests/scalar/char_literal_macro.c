// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0024 item 5: a literal written in a macro is the same node as one written in place
#include <cerastes.h>

#define DELETE_CHAR '\x7f'

int delete_char(void)
    CERASTES_ENSURES(r, r == 127)
{
  return DELETE_CHAR;
}

int use_twice(void)
    CERASTES_ENSURES(r, r == 254)
{
  return DELETE_CHAR + DELETE_CHAR;
}
