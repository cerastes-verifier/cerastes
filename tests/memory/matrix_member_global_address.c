// expect: verified
// expect: native-ok
// mode: c23, c++20
// twin-exempt: the must-fail form is the rejected pin negative/matrix_member_global_address_init_c.c; nothing in a function that only forms or passes an address can fail
// Pins: 0064 item 5 (edg-backend report 17 section E, probes pE/gu_const_arg.c, gu_sym_arg.c, gu_sym_assign.c): the address of a cell of a global structure's matrix member passed as an argument and assigned to a pointer verifies in C and in C++ (only the initializer form is the C-only rejection of negative/matrix_member_global_address_init_c.c)
// Pre-registered (test-mutator-10, Task 1/2, before any run): verifies (both modes)
#include <cerastes.h>
#include <stddef.h>

struct U {
  int n;
  int m[2][3];
};

static struct U gu = {0, {{1, 2, 3}, {4, 5, 6}}};

int use(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  return *p;
}

int by_arg_const(int i, int j)
    CERASTES_REQUIRES(cerastes_owns(gu) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(r, cerastes_owns(gu))
{
  (void)i;
  (void)j;
  return use(&gu.m[1][0]);
}

int by_arg_symbolic(int i, int j)
    CERASTES_REQUIRES(cerastes_owns(gu) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(r, cerastes_owns(gu))
{
  return use(&gu.m[i][j]);
}

int by_assign(int i, int j)
    CERASTES_REQUIRES(cerastes_owns(gu) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(r, cerastes_owns(gu))
{
  int *q;
  (void)i;
  (void)j;
  q = &gu.m[i][j];
  return q == NULL;
}
