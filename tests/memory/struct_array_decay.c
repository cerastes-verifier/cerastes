// expect: verified
// expect: native-ok
// mode: c23, c++20
// (a comment line keeps the line numbers of the program this test replaces)
// Pins: 0063 items 5 and 8 (flip A of M4.5e1e1, the staging pin struct_array_reject_decay.c retired): the decay of s.a passed as an int * (h and f, unchanged, line for line), and passed with its count to a callee's owns_range and reads_range (bound to a local first; the array member's other member is not given away)
// Pre-registered from record 0063 item 7 (Part C), before any run: verifies
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

int h(int *p) CERASTES_REQUIRES(cerastes_live(p)) CERASTES_ENSURES(cerastes_live(p)) { return p != NULL; }

int f(void)
{
  struct S s = {{1, 2, 3, 4}, 5};
  return h(s.a);
}

void fill(int *a, int n)
    CERASTES_REQUIRES(0 < n && cerastes_owns_range(a, n))
    CERASTES_ENSURES(cerastes_owns_range(a, n) && a[0] == 7)
{
  (void)n;
  a[0] = 7;
}

int peek(int *a, int n)
    CERASTES_REQUIRES(0 < n && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && r == a[0])
{
  (void)n;
  return a[0];
}

int use(void)
    CERASTES_ENSURES(r, r == 7)
{
  struct S s = {{1, 2, 3, 4}, 5};
  fill(s.a, 4);
  int v = peek(s.a, 4);
  CERASTES_CHECK(v == 7 && s.n == 5);
  return v;
}
