// expect: error postcondition @ tests/scalar/headers/bad_post.h:L7
// mode: c23, c++20
// twin: tests/scalar/header_use.c
// Pins: 0011 item 4: a postcondition failure in a header body is reported at the header's return
#include <cerastes.h>

#include "../headers/bad_post.h"

int use(int x)
    CERASTES_REQUIRES(x > 0)
{
  return one_h(x);
}
