// expect: rejected cerastes-unsupported_annotation @ L31
// expect: rejected cerastes-unsupported_annotation @ L44
// expect: rejected cerastes-unsupported_annotation @ L54
// expect: rejected cerastes-unsupported_annotation @ L68
// mode: c23
// c-only: restrict does not exist in C++20 (EDG, clang++ and g++ refuse it, decision 0037 item 2)
// twin: tests/memory/restrict_dropped.c
// Pins: 0064 item 19 (question 326), clause (iii): a restrict parameter in a function that calls a function that can hand the body a pointer reaching the pointee's field: a call's pointer result (probe c3c / e2: getg() ensures r == &g; the base verifies it, its twin fails), an int * result whose value is based on p (same(p): over-strict by design, C says it is based on p), an out-parameter (probe a2: getp(&q)) and a structure result with a pointer leaf (probe b: geth().q). Each is rejected at its function head; (iii) has no exemption [0064 item 19, question 326: every restrict parameter is now rejected by that one rule, whatever the contract; this pin's row keeps its tag and line and its condition no longer matters]
// Pre-registered from record 0064 item 19 (probes c3c, c3a, a2, b), before any run: rejected
#include <cerastes.h>

int g;

struct H {
  int *q;
};

int *getg(void)
    CERASTES_ENSURES(r, r == &g);

int *same(int *x)
    CERASTES_ENSURES(r, r == x);

void getp(int **out)
    CERASTES_REQUIRES(cerastes_owns(*out))
    CERASTES_ENSURES(cerastes_owns(*out) && *out == &g);

struct H geth(void)
    CERASTES_ENSURES(r, r.q == &g);

int call_result(int *restrict p)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == 2)
{
  int *q = getg();
  if (p == q) {
    *p = 1;
    *q = 2;
    return *p;
  }
  return 2;
}

int based_on_p(int *restrict p)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == 2)
{
  *p = 1;
  int *q = same(p);
  *q = 2;
  return *p;
}

int out_parameter(int *restrict p)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == 2)
{
  int *q = 0;
  getp(&q);
  if (p == q) {
    *p = 1;
    *q = 2;
    return *p;
  }
  return 2;
}

int structure_result(int *restrict p)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == 2)
{
  if (p == geth().q) {
    *p = 1;
    *geth().q = 2;
    return *p;
  }
  return 2;
}
