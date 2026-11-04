// expect: rejected cerastes-unsupported_annotation @ L11
// c-only: restrict does not exist in C++20 (EDG, clang++ and g++ refuse it, decision 0037 item 2)
// twin: tests/memory/struct_chain_contract.c
// Pins: 0045 item 13: a restrict parameter beside a struct pointer whose member chain owns(l->q->x) names: rejected at the function head
#include <cerastes.h>
struct L {
  struct L *q;
  int x;
};

int f(int *restrict p, struct L *l)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*l) && cerastes_owns(l->q->x) && l->q->x == 1)
    CERASTES_ENSURES(r, cerastes_owns(*p) && cerastes_owns(*l) && cerastes_owns(l->q->x) && r == 2)
{
  *p = 1;
  l->q->x = 2;
  return *p;
}
