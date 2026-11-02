// expect: error postcondition @ L17
// mode: c23, c++20
// twin: tests/memory/struct_field_update.c
// Pins: 0043 items 5, 9: the ensures says the counter grows by 1, but it grows by step
#include <cerastes.h>

struct counter {
  int n;
  int step;
};

void tick(struct counter *c)
    CERASTES_REQUIRES(cerastes_owns(*c) && 0 <= c->n && c->n <= 100 && 0 <= c->step && c->step <= 10)
    CERASTES_ENSURES(cerastes_owns(*c) && c->n == cerastes_old(c->n) + 1 && c->step == cerastes_old(c->step))
{
  c->n += c->step;
}  // <- here
