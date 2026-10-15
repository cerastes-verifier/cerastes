// expect: golden
// expect: native-ok
// invariant on while, for and do statements; the for counter is declared before the loop.
#include <cerastes.h>

// tag::while[]
int sum_to(int n) CERASTES_REQUIRES(n >= 0)
{
  int i = 0;
  int s = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && s >= 0) while (i < n) {
    s += i;
    i += 1;
  }
  return s;
}
// end::while[]

// tag::for[]
int count(int n) CERASTES_REQUIRES(n >= 0)
{
  int i;
  int c = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && c == i) for (i = 0; i < n; ++i) {
    c += 1;
  }
  return c;
}
// end::for[]

// tag::do[]
int at_least_one(int n) CERASTES_REQUIRES(n >= 0)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n + 1) do {
    i += 1;
  } while (i < n);
  return i;
}
// end::do[]
