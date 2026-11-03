// expect: verified
// expect: native-ok
// mode: c23, c++20
// An atom on one member: the callee owns p->count only, so the caller's p->limit keeps its value.
// tag::source[]
#include <cerastes.h>

struct counter {
  int count;
  int limit;
};

void reset(struct counter *p)
    CERASTES_REQUIRES(cerastes_owns(p->count))
    CERASTES_ENSURES(cerastes_owns(p->count) && p->count == 0)
{
  p->count = 0;
}

int limit_after_reset(void)
    CERASTES_ENSURES(r, r == 10)
{
  struct counter c = {.count = 3, .limit = 10};
  reset(&c);
  return c.count + c.limit;
}
// end::source[]
