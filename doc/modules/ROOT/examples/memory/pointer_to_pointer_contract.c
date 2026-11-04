// expect: verified
// expect: native-ok
// mode: c23, c++20
// A contract that names the object behind a pointer to a pointer: owns(*pp) holds the pointer, owns(**pp) the int it points to, live(*pp) says the pointer is usable; *pp == old(*pp) says the pointer is unchanged.
// tag::source[]
#include <cerastes.h>

void set(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns(**pp))
    CERASTES_ENSURES(cerastes_owns(*pp) && *pp == cerastes_old(*pp) && cerastes_owns(**pp) && **pp == 5)
{
  **pp = 5;
}

int same(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_live(*pp))
    CERASTES_ENSURES(r, cerastes_owns(*pp) && *pp == cerastes_old(*pp) && cerastes_live(*pp) && r == 1)
{
  int *q = *pp;
  return q == *pp;
}

int use(void)
    CERASTES_ENSURES(r, r == 5)
{
  int x = 0;
  int *p = &x;
  set(&p);
  CERASTES_CHECK(x == 5);
  return x;
}
// end::source[]
