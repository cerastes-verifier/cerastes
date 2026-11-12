// expect: rejected cerastes-unsupported_annotation @ L10
// mode: c23
// c-only: restrict does not exist in C++20 (decision 0037 item 2)
// twin: tests/memory/range_restrict.c
// Pins: 0060 item 14: a restrict parameter p beside another parameter of its pointee type with no atom (q): q could alias p, so the contract does not own what restrict promises: rejected at the function
// Pre-registered from record 0060 item 14 (Part E), before the code
#include <cerastes.h>
#include <stddef.h>

void f(int *restrict p, int *q, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n))
    CERASTES_ENSURES(cerastes_owns_range(p, n))
{
  (void)p;
  (void)q;
  (void)n;
}
