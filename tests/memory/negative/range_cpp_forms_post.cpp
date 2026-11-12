// expect: error postcondition @ L13
// mode: c++20
// twin: tests/memory/range_cpp_forms.cpp
// Pins: 0060 item 9: get claims r == 0 where the element is p[i] (the C++ spelling): the postcondition fails at the return
// Pre-registered from record 0060 item 9, before the code
#include <cerastes.h>

int get(int *p, int n, int i)
    CERASTES_REQUIRES(cerastes::owns_range(p, n) && 0 <= i && i < n)
    CERASTES_ENSURES(r, cerastes::owns_range(p, n) && r == 0)
{
  (void)n;
  return p[i];
}

int first(const int *p)
    CERASTES_REQUIRES(cerastes::reads_range(p, 2))
    CERASTES_ENSURES(r, cerastes::reads_range(p, 2) && r == p[0])
{
  return p[0];
}

int first_deref(const int *p)
    CERASTES_REQUIRES(cerastes::reads_range(p, 2))
{
  return *p;
}
