// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0047 item 7 (question 123): a path that ends in exit, abort or _Exit does not return, so it is not checked for leaks (exit is C's explicit way to end with allocations held)
// Pre-registered from record 0047 item 7: verifies; the paths end in exit (ensures(false)), no return row is generated for them
#include <cerastes.h>
#include <stdlib.h>

int main(void)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    exit(1);
  }
  *p = 3;
  exit(0);
}
