// expect: error permission @ L32
// mode: c23, c++20
// twin: tests/memory/range_struct_nested_cell.c
// Pins: 0064 item 7: the same call with the cell v[0].t.a, inside the range [0, 1): the pairing row of the nested leaf (the element index recovered by one parent_of(block_of(.)) per member step) fails at the call (permission, once), not Silicon's own precondition
// Pre-registered from record 0064 items 7, 8 and 10 (Part C1), before any run: permission at the call
#include <cerastes.h>
#include <stddef.h>

struct T {
  int a;
  int b;
};

struct S {
  int x;
  struct T t;
};

void nested(struct S *p, int n, int *q)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && cerastes_owns(*q))
    CERASTES_ENSURES(cerastes_reads_range(p, n) && cerastes_owns(*q) && *q == 5)
{
  (void)p;
  (void)n;
  *q = 5;
}

int use(void)
    CERASTES_ENSURES(r, r == 5)
{
  struct S v[2] = {{1, {2, 3}}, {4, {5, 6}}};
  nested(v, 1, &v[0].t.a);  // <- here
  return v[1].t.a;
}
