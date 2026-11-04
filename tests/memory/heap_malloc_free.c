// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0047 items 2, 4, 6, 7: malloc's typed view (one int, the size sizeof(int) as sizeof(T), sizeof *p or the constant 4), the null test, a write and a read of the cell, free, no leak
// Pre-registered from record 0047 items 2, 4, 6, 7 (the typed view and free): verifies in both modes; the C cast is the conversion in C and C++
#include <cerastes.h>
#include <stdlib.h>

int roundtrip(void)
    CERASTES_ENSURES(r, r == 7 || r == -1)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  *p = 7;
  int v = *p;
  free(p);
  return v;
}

int sized_by_object(void)
    CERASTES_ENSURES(r, r == 3 || r == -1)
{
  int *q = (int *)malloc(sizeof *q);
  if (q == NULL) {
    return -1;
  }
  *q = 3;
  int w = *q;
  free(q);
  return w;
}

int sized_by_constant(void)
    CERASTES_ENSURES(r, r == 2 || r == -1)
{
  int *q = (int *)malloc(4);
  if (q == NULL) {
    return -1;
  }
  *q = 2;
  int w = *q;
  free(q);
  return w;
}

int main(void)
{
  int a = roundtrip();
  int b = sized_by_object();
  int c = sized_by_constant();
  return (a == 7 && b == 3 && c == 2) ? 0 : 1;
}
