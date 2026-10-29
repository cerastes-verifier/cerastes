// expect: error pointer-comparison @ L12
// mode: c23, c++20
// twin: tests/memory/same_owned.c
// Pins: 0037 item 12: live alone allows a pointer one past its object's end, so p == q may compare pointers into different objects
#include <cerastes.h>
#include <stddef.h>

int same(int *p, int *q)
    CERASTES_REQUIRES(cerastes_live(p) && cerastes_live(q))
    CERASTES_ENSURES(r, cerastes_live(p) && cerastes_live(q))
{
  return p == q;
}
