// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0060 items 3 and 4 (audit N10): a bool range, an enumeration range (the type's range row) and a pointer-element range (WF of the stored pointer at the return rows)
// Pre-registered from record 0060 items 3 and 4, before the code: verifies
#include <cerastes.h>
#include <stddef.h>

enum Color { RED, GREEN };

bool first_flag(bool *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 0)
    CERASTES_ENSURES(r, cerastes_owns_range(p, n) && r == p[0])
{
  (void)n;
  return p[0];
}

enum Color first_color(enum Color *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 0)
    CERASTES_ENSURES(r, cerastes_owns_range(p, n) && r == p[0])
{
  (void)n;
  return p[0];
}

void clear_first(int **p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 0)
    CERASTES_ENSURES(cerastes_owns_range(p, n) && p[0] == NULL)
{
  (void)n;
  p[0] = NULL;
}
