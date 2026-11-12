// expect: error permission @ L23
// expect: error out-of-bounds @ L26
// expect: error permission @ L26
// mode: c23, c++20
// twin: tests/memory/range_element_types.c
// Pins: 0060 items 4 and 5: without n > 0 the enumeration range may be empty: the ensures reads p[0] without owning it (permission at the clause, in f__wf) and the body's p[0] fails its bound and permission rows
// Pre-registered from record 0060 items 4 and 5, before the code
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
    CERASTES_REQUIRES(cerastes_owns_range(p, n))
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
