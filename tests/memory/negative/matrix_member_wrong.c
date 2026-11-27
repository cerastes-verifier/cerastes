// expect: error postcondition @ L18
// mode: c23, c++20
// twin: tests/memory/matrix_member.c
// Pins: 0064 item 7: the member cell read back through the pointer is the value written, seven, so a result of eight fails the postcondition at the return
// Pre-registered from record 0064 item 7 (Part E), before any run: the one error
#include <cerastes.h>

struct U {
  int m[2][3];
  int n;
};

int through(struct U *p, int i, int j)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == 7)
{
  p->m[i][j] = 7;
  return p->m[i][j] + 1;  // <- here
}
