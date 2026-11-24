// expect: rejected cerastes-forall_function @ L11
// twin: tests/memory/forall_requires_read.c
// Pins: 0061 item 3 (audit round 1 S5): a lambda (enk_lambda) is not `f` or `&f`: forall_function
// Pre-registered from record 0061 item 3 (Part B), before the code
#include <cerastes.h>
#include <stddef.h>



void use(const int *a, int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && cerastes_forall([](int i, const int *b, int m) { return !(0 <= i && i < m) || b[i] >= 0; }, a, n))
    CERASTES_ENSURES(cerastes_reads_range(a, n))
{
  (void)a;
  (void)n;
}
