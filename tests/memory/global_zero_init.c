// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0041 items 3 and 4: static storage without an initializer is zero (an int, a bool, a null pointer), known to main in both languages (internal linkage)
#include <cerastes.h>
#include <stddef.h>

static int z;
static bool b;
static int *q;

int main(void)
{
  CERASTES_CHECK(z == 0 && !b && q == NULL);
  return z + (b ? 1 : 0) + (q != NULL ? 1 : 0);
}
