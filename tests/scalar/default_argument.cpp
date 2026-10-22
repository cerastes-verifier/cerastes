// expect: verified
// expect: native-ok
// Pins: 0012 item 6 (P7): a default argument is an explicit node at the call and is checked there (C++)
#include <cerastes.h>

int add_default(int a, int b = 10)
    CERASTES_REQUIRES(a >= 0 && a <= 1000 && b >= 0 && b <= 1000)
    CERASTES_ENSURES(r, r == a + b)
{
  return a + b;
}

int use(void)
    CERASTES_ENSURES(r, r == 11)
{
  return add_default(1);
}

// Both ends of the callee's range, through the explicit arguments: a stronger requires of `add_default` fails here.
int use_edges(void)
    CERASTES_ENSURES(r, r == 2000)
{
  int low = add_default(0, 1000);
  int high = add_default(1000, 0);
  return low + high;
}
