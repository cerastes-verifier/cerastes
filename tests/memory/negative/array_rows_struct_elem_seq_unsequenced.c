// expect: error unsequenced @ L18
// mode: c23, c++20
// twin: tests/memory/array_rows_struct_elem_seq.c
// Pins: 0064 item 11 (0039 item 3): without (i != k || j != l) the write to v[i].a[j] and the increment of v[k].a[l] may reach one cell of one element (the path-precise row)
// Pre-registered from record 0064 item 11 (Part E), before any run: the one error
#include <cerastes.h>

struct W {
  int a[4];
  int n;
};

int cells(int i, int j, int k, int l)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 4 && 0 <= k && k < 2 && 0 <= l && l < 4)
{
  struct W v[2] = {{{1, 2, 3, 4}, 5}, {{6, 7, 8, 9}, 10}};
  int old = v[k].a[l];
  v[i].a[j] = v[k].a[l]++;  // <- here
  CERASTES_CHECK(v[i].a[j] == old && v[k].a[l] == old + 1);
  return v[i].a[j] - old;
}
