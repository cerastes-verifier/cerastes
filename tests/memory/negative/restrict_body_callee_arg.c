// expect: rejected cerastes-unsupported_annotation @ L59
// expect: rejected cerastes-unsupported_annotation @ L68
// expect: rejected cerastes-unsupported_annotation @ L77
// expect: rejected cerastes-unsupported_annotation @ L87
// expect: rejected cerastes-unsupported_annotation @ L96
// mode: c23
// c-only: restrict does not exist in C++20 (EDG, clang++ and g++ refuse it, decision 0037 item 2)
// twin: tests/memory/restrict_dropped.c
// Pins: 0064 item 19 (question 326; audit round 2), clause (iv): a restrict parameter p passed to a callee: whose BODY names the global and tests x == &g (probe b1; the base verified it, with an ensures true only of the aliased value), whose body calls a function that owns the global (b3, a callee's callee), whose body only reads the global (b5), that has no body (b4), and the over-strict shape set(p, 3), a helper that only writes through its parameter (listed in the record: every call that passes p on is rejected). Each is rejected at its function head, as clauses (i) to (iii); the contract of the callee never reaches the global of the caller [0064 item 19, question 326: every restrict parameter is now rejected by that one rule, whatever the contract; this pin's row keeps its tag and line and its condition no longer matters]
// Pre-registered from record 0064 item 19 as revised by audit round 2 (probes b1, b3, b4, b5), before any run: rejected (tag as restrict_body_callee_global.c)
#include <cerastes.h>

int g;

static void body_names(int *x)
    CERASTES_REQUIRES(cerastes_owns(*x))
    CERASTES_ENSURES(cerastes_owns(*x) && (x != &g || *x == 2))
{
  if (x == &g) {
    g = 2;
  }
}

static void seth(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns(g) && g == 2)
{
  g = 2;
}

static void deep(int *x)
    CERASTES_REQUIRES(cerastes_owns(*x) && x == &g)
    CERASTES_ENSURES(cerastes_owns(*x) && *x == 2)
{
  seth();
}

static int only_reads(int *x)
    CERASTES_REQUIRES(cerastes_reads(*x))
    CERASTES_ENSURES(r, cerastes_reads(*x) && r == *x)
{
  if (x == &g) {
    return g;
  }
  return *x;
}

void bodiless(int *x)
    CERASTES_REQUIRES(cerastes_owns(*x))
    CERASTES_ENSURES(cerastes_owns(*x) && (x != &g || *x == 2));

void set(int *x, int v)
    CERASTES_REQUIRES(cerastes_owns(*x))
    CERASTES_ENSURES(cerastes_owns(*x) && *x == v)
{
  *x = v;
}

int callee_body(int *restrict p)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p) && p == &g)
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == 2)
{
  *p = 1;
  body_names(p);
  return *p;
}

int callee_callee(int *restrict p)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p) && p == &g)
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == 2)
{
  *p = 1;
  deep(p);
  return *p;
}

int callee_reads(int *restrict p)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p) && p == &g)
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == 3)
{
  *p = 1;
  int a = only_reads(p);
  *p = 2;
  return a + *p;
}

int callee_bodiless(int *restrict p)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p) && p == &g)
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == 2)
{
  *p = 1;
  bodiless(p);
  return *p;
}

int callee_set(int *restrict p)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == 3)
{
  set(p, 3);
  return *p;
}
