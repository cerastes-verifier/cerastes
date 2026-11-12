// expect: verified
// expect: native-ok
// mode: c++20
// Pins: 0060 items 2 and 3: the C++ spellings cerastes::owns_range and cerastes::reads_range (the latter declared with owns_range's signature), a literal count, and a reads_range over a const pointee (first_deref is the program of reject_owns_range_pointer.cpp with reads_range: its positive, flip B)
// Pre-registered from record 0060 items 2 and 3, before the code: verifies
#include <cerastes.h>

int get(int *p, int n, int i)
    CERASTES_REQUIRES(cerastes::owns_range(p, n) && 0 <= i && i < n)
    CERASTES_ENSURES(r, cerastes::owns_range(p, n) && r == p[i])
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
