// expect: rejected cerastes-unsupported_type @ L17
// expect: rejected cerastes-forall_function @ L20
// mode: c23, c++20
// twin: tests/memory/forall_requires_read.c
// Pins: 0061 item 3 (audit round 1 S5): a function pointer variable is not `f` or `&f`: forall_function at the quantifier. No C shape reaches that rule without a second rejection: the declaration of any function pointer (a parameter, a local, a global) is itself unsupported_type, so the pin lists both (a function pointer PARAMETER of the enclosing function stops that function at its signature before the quantifier is read)
// Pre-registered from record 0061 item 3 (Part B), before the code
#include <cerastes.h>
#include <stddef.h>

bool other(int i, const int *a, int n)
{
  (void)i;
  (void)a;
  return n > 0;
}

bool (*gfp)(int, const int *, int) = other;

void use(const int *a, int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && cerastes_forall(gfp, a, n))
    CERASTES_ENSURES(cerastes_reads_range(a, n))
{
  (void)a;
  (void)n;
}
