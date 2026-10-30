// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0040 item 2: &p of a pointer parameter (0037 item 4's copy); the contract keeps reading the entry value of p
#include <cerastes.h>
#include <stddef.h>

int via_param(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && *p < 100)
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == cerastes_old(*p) + 1)
{
  int **pp = &p;
  **pp = **pp + 1;
  return **pp;
}
