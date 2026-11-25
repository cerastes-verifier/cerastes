// expect: error postcondition @ L19
// mode: c23, c++20
// twin: tests/memory/struct_array_owns.c
// Pins: 0063 item 7: the element grew by 1 and the contract claims 2 (an element read under old): postcondition at the closing brace
// Pre-registered from record 0063 item 7 (Part C), before any run: the one error
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

void bump(struct S *p, int i)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= i && i < 4 && p->a[i] < 100)
    CERASTES_ENSURES(cerastes_owns(*p) && p->a[i] == cerastes_old(p->a[i]) + 2 && p->n == cerastes_old(p->n))
{
  p->a[i] += 1;
}  // <- here
