// expect: rejected cerastes-array_address_null_test @ L31
// expect: rejected cerastes-unsupported_expression @ L37
// expect: rejected cerastes-unsupported_annotation @ L41
// expect: rejected cerastes-unsupported_annotation @ L47
// expect: rejected cerastes-unsupported_annotation @ L52
// expect: rejected cerastes-unsupported_annotation @ L57
// mode: c23, c++20
// twin: tests/memory/global_array_declared.c
// Pins: 0063 items 1, 7 and 9: a null comparison of an array global's address, g == NULL (clang -Wtautological-pointer-compare and GCC -Waddress; `if (g)` and `!g` draw EDG's constant-condition warning in C only, so the test uses the comparison), g + i in code (M4.5e2), owns_range(g, n) and an atom on &g[1] in a contract (M4.5e2), and cerastes_forall and a pure function over a global array IN A CONTRACT (M4.5e1e2: only a check takes a named global array): one rejection each (the tags of the contract rows are unsupported_annotation by the pins of the same kind; to confirm at the joined run)
// Pre-registered from record 0063 item 2 (Part B), before any run: rejected
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool nonneg(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || a[i] >= 0;
}

CERASTES_PURE int first(const int *a, int n)
    CERASTES_REQUIRES(0 < n && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, -1000 <= r && r <= 1000)
{
  // exactly one return, and a body that reads n
  return n > 0 ? (a[0] > 1000 ? 1000 : (a[0] < -1000 ? -1000 : a[0])) : 0;
}

static int g[4] = {1, 2, 3, 4};

int truth(void)
{
  return g == NULL;  // <- here
}

int *plus(int i)
    CERASTES_REQUIRES(0 <= i && i < 4)
{
  return g + i;  // <- here
}

void ranged(int n)
    CERASTES_REQUIRES(cerastes_owns_range(g, n))  // <- here
{
  (void)n;
}

void element(void)
    CERASTES_REQUIRES(cerastes_owns(&g[1]))  // <- here
{
}

void quantified(void)
    CERASTES_REQUIRES(cerastes_owns(g) && cerastes_forall(nonneg, g, 4))  // <- here
{
}

void applied(void)
    CERASTES_REQUIRES(cerastes_owns(g) && first(g, 4) <= 1000)  // <- here
{
}
