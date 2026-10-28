// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0032 items 3, 7: NULL, 0 and nullptr (C23 and C++) are null pointer constants; truth tests, == and !=
#include <stddef.h>
#include <cerastes.h>

int spellings(void)
    CERASTES_ENSURES(r, r == 8)
{
  int x = 1;
  int *a = NULL;
  int *b = 0;
  int *c = nullptr;
  int *d = &x;
  int n = 0;
  if (!a) {
    n += 1;
  }
  if (!b) {
    n += 1;
  }
  if (!c) {
    n += 1;
  }
  if (a == NULL) {
    n += 1;
  }
  if (b == 0) {
    n += 1;
  }
  if (c == nullptr) {
    n += 1;
  }
  if (d != NULL) {
    n += 1;
  }
  if (d) {
    n += 1;
  }
  return n;
}
