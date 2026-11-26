// expect: error null-dereference @ L17
// expect: error lifetime @ L17
// mode: c23, c++20
// twin: tests/memory/array_struct_subscript.c
// Pins: 0064 item 5 (audit round 2, issue 6; the lowering's report 3, question 277): p[i].x through a pointer parameter that no atom owns reads a cell nothing grants: the null and liveness rows fail at the access and nothing after them is reported (Silicon assumes the failed liveness permission, which the heap lacks), so the twin lists both rows at the one line
// Pre-registered from record 0064 item 5, the lowering's report 3, before any run: the two errors at the access
#include <cerastes.h>

struct P {
  int x;
  int y;
};

int unowned(struct P *p, int i)
    CERASTES_REQUIRES(i == 0)
{
  return p[i].x;  // <- here
}
