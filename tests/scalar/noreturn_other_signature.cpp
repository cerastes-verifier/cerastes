// expect: rejected cerastes-annotation_reserved_library @ L8
// expect: native-ok
// Pins: 0046 item 6 and question 117 (was 0015 item 5: exit(void) is not exit(int), an ordinary function; C++ only): the global name exit is reserved whatever the signature, so its user contract is rejected
// A C++ program may not declare ::exit with a contract of its own, whatever its parameter list.
#include <cerastes.h>

void exit(void)
    CERASTES_ENSURES(true);

int use(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
    CERASTES_ENSURES(r, r == x)
{
  if (x > 5) {
    exit();
  }
  return x;
}
