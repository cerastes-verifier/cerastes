// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 15's last clause and question 348 (a structure whose array member holds structures stays rejected at Part D's six positions; everything Part C2 accepts for it keeps its outcome): owns(*p) of struct T with a member array of structures, a callee that writes p->v[1].x and p->n, the caller's local; the program of struct_array_reject_aggregate_members.c's neighbours that must still verify. The rejection pin is negative/struct_array_reject_aggregate_members.c
// Pre-registered from record 0064 item 15's last clause and the precedent struct_member_array_struct_owns.c, before any run: verifies on the frozen build too (it is Part C2's)
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct P {
  int x;
  int y;
};

struct T {
  struct P v[2];
  int n;
};

void set(struct T *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && p->v[1].x == 9 && p->n == 1)
{
  p->v[1].x = 9;
  p->n = 1;
}

int use(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct T t = {{{1, 2}, {3, 4}}, 5};
  set(&t);
  CERASTES_CHECK(t.v[1].x == 9 && t.n == 1);
  return 0;
}
