// expect: rejected cerastes-unsupported_annotation @ L20
// mode: c23, c++20
// twin: tests/memory/forall_requires_read.c
// Pins: 0061 question 259 (fail-closed narrowing, lifted by M4.5e2): pointer-typed elements under the binder in a check are unsupported_annotation (in a contract they are accepted: forall_pointer_elements.c)
// Pre-registered from record 0061 item 10 (Part B), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool all_null(int i, int **a, int n)
{
  return !(0 <= i && i < n) || a[i] == NULL;
}

void use(int **p)
    CERASTES_REQUIRES(cerastes_owns_range(p, 2))
    CERASTES_ENSURES(cerastes_owns_range(p, 2))
{
  p[0] = NULL;
  p[1] = NULL;
  CERASTES_CHECK(cerastes_forall(all_null, p, 2));
}
