// expect: error division-by-zero @ L18
// expect: error division-by-zero @ L27
// expect: error division-by-zero @ L36
// expect: error division-by-zero @ L45
// mode: c23, c++20
// twin: tests/memory/array_rows_struct_elem_wf.c
// Pins: 0064 items 7 and 14 (as range_struct_wf_division.c, for an element's array member; f__wf's ghosts, one per leaf path and slot): each function proves p[0].a[1] != 0 (or p[i].a[j] != 0) in its requires and divides by a different cell in its ensures. other_cell: p[0].a[2], another cell of the same member; other_elem: p[1].a[1], the same cell of another element; owned: under owns_range the body may change p[0].a[1]; other_index: p[k].a[l] against p[i].a[j]
// Pre-registered from record 0064 item 7 (Part E), before any run: division-by-zero at the ensures line of each function
#include <cerastes.h>

struct W {
  int a[4];
  int n;
};

int other_cell(struct W *p, int n)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && n > 1 && p[0].a[1] != 0)
    CERASTES_ENSURES(r, cerastes_reads_range(p, n) && (r == 0 || r == 10 / p[0].a[2]))  // <- here
{
  (void)p;
  (void)n;
  return 0;
}

int other_elem(struct W *p, int n)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && n > 1 && p[0].a[1] != 0)
    CERASTES_ENSURES(r, cerastes_reads_range(p, n) && (r == 0 || r == 10 / p[1].a[1]))  // <- here
{
  (void)p;
  (void)n;
  return 0;
}

int owned(struct W *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 1 && p[0].a[1] != 0)
    CERASTES_ENSURES(r, cerastes_owns_range(p, n) && (r == 0 || r == 10 / p[0].a[1]))  // <- here
{
  (void)n;
  p[0].a[1] = 0;
  return 0;
}

int other_index(struct W *p, int n, int i, int j, int k, int l)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && 0 <= i && i < n && 0 <= j && j < 4 && 0 <= k && k < n && 0 <= l && l < 4 && p[i].a[j] != 0)
    CERASTES_ENSURES(r, cerastes_reads_range(p, n) && (r == 0 || r == 10 / p[k].a[l]))  // <- here
{
  (void)p;
  (void)n;
  (void)i;
  (void)j;
  (void)k;
  (void)l;
  return 0;
}
