// expect: error postcondition @ L15
// mode: c23, c++20
// twin: tests/memory/range_old_elem.c
// Pins: 0060 item 5: without j != i the store may hit p[j]: p[j] == cerastes_old(p[j]) fails at the closing brace
// Pre-registered from record 0060 item 5, before the code
#include <cerastes.h>

void set(int *p, int n, int i, int j, int v)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && 0 <= i && i < n && 0 <= j && j < n)
    CERASTES_ENSURES(cerastes_owns_range(p, n) && p[i] == v && p[j] == cerastes_old(p[j]))
{
  (void)n;
  (void)j;
  p[i] = v;
}
