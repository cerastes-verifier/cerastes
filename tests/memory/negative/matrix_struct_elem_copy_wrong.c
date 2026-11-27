// expect: error postcondition @ L19
// mode: c23, c++20
// twin: tests/memory/matrix_struct_elem_copy.c
// Pins: 0064 item 12: after v[i] = v[j] the elements agree (the difference is zero), so a postcondition of one fails at the return
// Pre-registered from record 0064 item 12 (Part E), before any run: the one error
#include <cerastes.h>

struct W {
  int a[4];
  int n;
};

int copy(int i, int j)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 2)
    CERASTES_ENSURES(r, r == 1)
{
  struct W v[2] = {{{1, 2, 3, 4}, 5}, {{6, 7, 8, 9}, 10}};
  v[i] = v[j];
  return (v[i].a[2] - v[j].a[2]) + (v[i].n - v[j].n);  // <- here
}
