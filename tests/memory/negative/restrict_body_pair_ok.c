// expect: rejected cerastes-unsupported_annotation @ L22
// expect: rejected cerastes-unsupported_annotation @ L30
// expect: rejected cerastes-unsupported_annotation @ L40
// expect: rejected cerastes-unsupported_annotation @ L50
// expect: rejected cerastes-unsupported_annotation @ L59
// mode: c23
// c-only: restrict does not exist in C++20 (EDG, clang++ and g++ refuse it, decision 0037 item 2)
// twin: tests/memory/restrict_dropped.c
// Pins: 0064 item 19 (question 326, the owner): every restrict parameter is rejected, unsupported_annotation, once per function at its head; drop the keyword to verify the function (tests/memory/restrict_dropped.c). Re-expected from verified by Flips R. It verified before as: 0064 item 19 (question 326; audit round 2): the shapes the pair rule and clause (iv) keep accepted. A restrict function that calls a helper taking only scalars (no pointer reaches the pointee's field: clause (iv) does not apply); owns(*p) beside owns_range(q, n) with n > 0 as a top-level conjunct and NO q == p (with n > 0 the alias is excluded by the permissions; a requires q == p would be unsatisfiable); the same for restrict p backed by owns_range(p, n) beside owns(*q); a literal count (owns_range(q, 2)); and owns(h) of a global array int h[4] beside owns(*p) with the body naming h[1] (a whole-array atom is surely non-empty: the exemption for a held global). Each body's accesses are in range and the model is C's
// Pre-registered from record 0064 item 19 (Part R, Flips R), before the run: one row per function with a restrict parameter, at its head line
#include <cerastes.h>

int h[4];

int twice(int a)
    CERASTES_REQUIRES(0 <= a && a < 100)
    CERASTES_ENSURES(r, r == 2 * a)
{
  return 2 * a;
}

int scalar_helper(int *restrict p)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p) && *p == 4 && r == 4)
{
  *p = twice(2);
  return *p;
}

int cell_and_range(int *restrict p, int *q, int n)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns_range(q, n) && n > 0)
    CERASTES_ENSURES(r, cerastes_owns(*p) && cerastes_owns_range(q, n) && r == 1)
{
  (void)n;
  *p = 1;
  q[0] = 2;
  return *p;
}

int range_and_cell(int *restrict p, int n, int *q)  // <- here
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && cerastes_owns(*q) && n > 0)
    CERASTES_ENSURES(r, cerastes_owns_range(p, n) && cerastes_owns(*q) && r == 1)
{
  (void)n;
  p[0] = 1;
  *q = 2;
  return p[0];
}

int literal_count(int *restrict p, int *q)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns_range(q, 2))
    CERASTES_ENSURES(r, cerastes_owns(*p) && cerastes_owns_range(q, 2) && r == 1)
{
  *p = 1;
  q[1] = 2;
  return *p;
}

int global_array(int *restrict p)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(h))
    CERASTES_ENSURES(r, cerastes_owns(*p) && cerastes_owns(h) && r == 3)
{
  *p = 1;
  h[1] = 2;
  return *p + h[1];
}

int main(void)
{
  int a = 0;
  int r1 = scalar_helper(&a);
  int c = 0;
  int b[2] = {0, 0};
  int r2 = cell_and_range(&c, b, 2);
  int d[2] = {0, 0};
  int e = 0;
  int r3 = range_and_cell(d, 2, &e);
  int k = 0;
  int w[2] = {0, 0};
  int r4 = literal_count(&k, w);
  int m = 0;
  int r5 = global_array(&m);
  return r1 + r2 + r3 + r4 + r5 - 10;
}
