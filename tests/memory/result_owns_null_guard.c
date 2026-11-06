// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0052 item 2 (audit B1): ensures(r, r == NULL || (owns(*r) && r == &g)): the atom's guard is r != NULL, so the NULL path owes no amount (o is write there) and the other path gives the cell it holds; the caller tests the result before writing
// Pre-registered from record 0052 item 2: verifies in both modes (the NULL path used to fail none < o and exit 3)
#include <cerastes.h>
#include <stddef.h>

static int g = 3;

int *maybe(int c)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(r, r == NULL || (cerastes_owns(*r) && r == &g))
{
  if (c != 0) {
    return &g;
  }
  return NULL;
}

int main(void)
{
  int *p = maybe(1);
  int v = 0;
  if (p != NULL) {
    *p = 4;
    v = *p;
  }
  return v - v;
}
