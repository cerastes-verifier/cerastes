// expect: error permission @ L18
// c-only: a compound literal's address is an lvalue object in C; C++ has no such object
// twin: tests/memory/owning_field_complit.c
// Pins: 0055 items 4, 6 (audit round 2 (k)): the compound-literal object's owning field receives a pointer that is only live: "owning field `(struct buf){...}.data` receives a pointer that is not the start of a block the function holds" (w_complit_member.c)
// Pre-registered from audit round 2 on Part A (record 0055 as revised, 78838ca), w_complit_member.c: permission at the literal
#include <cerastes.h>
#include <stddef.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int f(int *a)
    CERASTES_REQUIRES(cerastes_live(a))
    CERASTES_ENSURES(r, cerastes_live(a) && r == 4)
{
  return (&(struct buf){ a, 4 })->n;  // <- here
}
