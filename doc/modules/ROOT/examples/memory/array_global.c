// expect: verified
// expect: native-ok
// mode: c23, c++20
// A global array with a partial braced initializer (the elements it does not list are zero): a function that owns it
// writes an element, one that reads it reads one, a loop in main carries it, and it is passed as a range.
// tag::source[]
#include <cerastes.h>

static int table[4] = {10, 20};

void store(int i, int v)
    CERASTES_REQUIRES(cerastes_owns(table) && 0 <= i && i < 4)
    CERASTES_ENSURES(cerastes_owns(table) && table[i] == v)
{
  table[i] = v;
}

int load(int i)
    CERASTES_REQUIRES(cerastes_reads(table) && 0 <= i && i < 4)
    CERASTES_ENSURES(r, cerastes_reads(table) && r == table[i])
{
  return table[i];
}

void clear(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 0)
    CERASTES_ENSURES(cerastes_owns_range(p, n) && p[0] == 0)
{
  (void)n;
  p[0] = 0;
}

int main(void)
{
  CERASTES_CHECK(table[1] == 20 && table[2] == 0);
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 4)
  CERASTES_DECREASES(4 - i)
  while (i < 4) {
    table[i] = i;
    i += 1;
  }
  store(3, 7);
  int t = load(3);
  CERASTES_CHECK(t == 7);
  (void)t;
  clear(table, 4);
  CERASTES_CHECK(table[0] == 0);
  return 0;
}
// end::source[]
