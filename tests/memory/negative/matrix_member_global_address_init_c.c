// expect: rejected cerastes-unsupported_type @ L21
// expect: rejected cerastes-unsupported_type @ L29
// c-only: the front end folds &gu.m[1][0] into an address constant in C only; the C++ twin is matrix_member_global_address_init.cpp
// twin: tests/memory/matrix_member_global_address.c
// Pins: 0064 item 5 and the note of audit round E3a (edg-backend report 17 section E, probes pE/gu_const_init.c and gu_sym_init.c): `int *q = &gu.m[1][0];` and `int *q = &gu.m[i][j];` as an INITIALIZER of a global structure's matrix member are rejected in C at the declaration (`unsupported_type`: the front end folds the address into a row-typed constant, which the encoder cannot read); in C++ they verify (the .cpp twin), and the argument and assignment forms verify in both modes (matrix_member_global_address.c). Listed, M4.8
// Pre-registered (test-mutator-10, Task 1/2, before any run): rows registered from report 17 section E (`cerastes-unsupported_type` at the declaration of q, C only; the C++ run of the same program is the positive matrix_member_global_address_init.cpp)
#include <cerastes.h>
#include <stddef.h>

struct U {
  int n;
  int m[2][3];
};

static struct U gu = {0, {{1, 2, 3}, {4, 5, 6}}};

int f_const(void)
    CERASTES_REQUIRES(cerastes_owns(gu))
    CERASTES_ENSURES(r, cerastes_owns(gu))
{
  int *q = &gu.m[1][0];  // <- here
  return q == NULL;
}

int f_symbolic(int i, int j)
    CERASTES_REQUIRES(cerastes_owns(gu) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(r, cerastes_owns(gu))
{
  int *q = &gu.m[i][j];  // <- here
  (void)i;
  (void)j;
  return q == NULL;
}
