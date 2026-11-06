// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0052 item 2 (given): the callee keeps its requires owns(p->x) (0037 item 13) and gives the cell through *r: owns(*r) has no partner; a contract cannot spell &p->x (decision 0012), so the caller learns nothing about where *q is and writes and reads it; the struct is a global so that no scope exit of the caller needs the liveness back
// Pre-registered from record 0052 item 2: verifies in both modes
#include <cerastes.h>

struct S {
  int x;
  int y;
};

static struct S gs = {1, 2};

int *member(struct S *p)
    CERASTES_REQUIRES(cerastes_owns(p->x))
    CERASTES_ENSURES(r, cerastes_owns(*r))
{
  return &p->x;
}

int main(void)
{
  int *q = member(&gs);
  *q = 7;
  int v = *q;
  CERASTES_CHECK(v == 7);
  return v - 7;
}
