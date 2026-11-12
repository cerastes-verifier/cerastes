// expect: error postcondition @ L15
// mode: c++20
// twin: tests/memory/range_old_elem.cpp
// Pins: 0060 item 5: the C++ spelling without j != i: p[j] == cerastes::old(p[j]) fails at the closing brace
// Pre-registered from record 0060 item 5, before the code
#include <cerastes.h>

void set(int *p, int n, int i, int j, int v)
    CERASTES_REQUIRES(cerastes::owns_range(p, n) && 0 <= i && i < n && 0 <= j && j < n)
    CERASTES_ENSURES(cerastes::owns_range(p, n) && p[i] == v && p[j] == cerastes::old(p[j]))
{
  (void)n;
  (void)j;
  p[i] = v;
}
