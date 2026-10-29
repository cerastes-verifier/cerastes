// expect: error postcondition @ L34
// mode: c23, c++20
// twin: tests/memory/pointer_params_types.c
// Pins: 0037 item 10: bump leaves the value, the contract says +1
#include <cerastes.h>
#include <stddef.h>

#include <stdbool.h>
enum color { RED, GREEN, BLUE };

void flip(bool *b)
    CERASTES_REQUIRES(cerastes_owns(*b))
    CERASTES_ENSURES(cerastes_owns(*b) && *b == !cerastes_old(*b))
{
  *b = !*b;
}

void next(enum color *c)
    CERASTES_REQUIRES(cerastes_owns(*c) && *c == RED)
    CERASTES_ENSURES(cerastes_owns(*c) && *c == GREEN)
{
  if (*c == RED) {
    *c = GREEN;
  } else {
    *c = BLUE;
  }
}

void bump(unsigned char *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && *p < 255)
    CERASTES_ENSURES(cerastes_owns(*p) && *p == cerastes_old(*p) + 1)
{
  *p = (unsigned char)*p;
}

int use_types(void)
    CERASTES_ENSURES(r, r == 8)
{
  bool b = true;
  flip(&b);
  enum color c = RED;
  next(&c);
  unsigned char u = 7;
  bump(&u);
  return b ? 0 : (c == GREEN ? u : 0);
}
