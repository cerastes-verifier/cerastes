// expect: verified
// expect: native-ok
// mode: c++20
// twin-exempt: the same text is the C-only rejected pin negative/matrix_member_global_address_init_c.c; nothing in a function that only forms an address can fail
// Pins: 0064 item 5 (edg-backend report 17 section E): the initializer forms of the C-only rejections in negative/matrix_member_global_address_init_c.c verify in C++ (`int *q = &gu.m[1][0];` and `int *q = &gu.m[i][j];`, an address of a cell of a global structure's matrix member, the front end not folding it there)
// Pre-registered (test-mutator-10, Task 1/2, before any run): verifies (C++ only; the same text in C is the rejected pin)
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
  int *q = &gu.m[1][0];
  return q == NULL;
}

int f_symbolic(int i, int j)
    CERASTES_REQUIRES(cerastes_owns(gu) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(r, cerastes_owns(gu))
{
  int *q = &gu.m[i][j];
  (void)i;
  (void)j;
  return q == NULL;
}
