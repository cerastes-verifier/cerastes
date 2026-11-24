// expect: error postcondition @ L21
// mode: c23, c++20
// twin: tests/memory/forall_ensures_three.c
// Pins: 0061 item 7 (S45 (a) twin): the ensures says every element is below 3 but a[2] is 3: the postcondition row of the quantified clause fails at the closing brace
// Pre-registered from record 0061 item 7 (Part B), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool below3(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || a[i] < 3;
}

void fill3(int *a)
    CERASTES_REQUIRES(cerastes_owns_range(a, 3))
    CERASTES_ENSURES(cerastes_owns_range(a, 3) && cerastes_forall(below3, a, 3))
{
  a[0] = 1;
  a[1] = 2;
  a[2] = 3;
}
