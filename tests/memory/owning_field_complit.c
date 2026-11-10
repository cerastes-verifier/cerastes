// expect: verified
// expect: native-ok
// c-only: a compound literal's address is an lvalue object in C; C++ has no such object
// Pins: 0055 items 4, 6 (audit round 2 (k)): an owning field of a compound-literal object &(struct buf){ ... } receives NULL: no value-rule error
// Pre-registered from audit round 2 on Part A (record 0055 as revised, 78838ca), w_complit_member.c with NULL in place of a live pointer: verifies in C
#include <cerastes.h>
#include <stddef.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int f(void)
    CERASTES_ENSURES(r, r == 4)
{
  return (&(struct buf){ NULL, 4 })->n;
}
