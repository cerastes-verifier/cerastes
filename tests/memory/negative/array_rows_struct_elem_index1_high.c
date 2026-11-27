// expect: error out-of-bounds @ L18
// mode: c23, c++20
// twin: tests/memory/array_rows_struct_elem.c
// Pins: 0064 item 5: v[i].a[j] with the first index one past its bound reaches no element (the array of structures's own length): out-of-bounds at the first access (the first bound row precedes the second and the cell's), nothing else on the path
// Pre-registered from record 0064 item 5 (Part E), before any run: the one error
#include <cerastes.h>

struct W {
  int a[4];
  int n;
};

int local(int i, int j)
    CERASTES_REQUIRES(0 <= i && i <= 2 && 0 <= j && j < 4)
    CERASTES_ENSURES(r, r == 7)
{
  struct W v[2];
  v[i].a[j] = 7;  // <- here
  return v[i].a[j];
}
