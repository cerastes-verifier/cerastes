// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0052 item 2 (the static local, questions 80, 157): ensures(r, live(r)) of a function returning &s is covered by the implicit owns(s): the caller may copy, compare and pass the pointer to a live-requiring function (0028 item 3a); with ensures(r, owns(*r)) the caller also writes and reads *r through its own implicit atom on the static
// Pre-registered from record 0052 item 2: verifies in both modes (lifts static_local_return_address_use.c's false alarm for a function that writes the clause)
#include <cerastes.h>
#include <stddef.h>

int *where_live(void)
    CERASTES_ENSURES(r, cerastes_live(r))
{
  static int s = 5;
  return &s;
}

int *where_owns(void)
    CERASTES_ENSURES(r, cerastes_owns(*r))
{
  static int t = 6;
  return &t;
}

int take_live(int *q)
    CERASTES_REQUIRES(cerastes_live(q))
    CERASTES_ENSURES(r, cerastes_live(q) && r == 0)
{
  (void)q;
  return 0;
}

int main(void)
{
  int *p = where_live();
  int *q = p;
  if (q == NULL) {
    return 1;
  }
  int z = take_live(p);
  int *w = where_owns();
  *w = 3;
  int v = *w;
  CERASTES_CHECK(v == 3);
  return z + (v - 3);
}
