// expect: error permission @ L12
// mode: c23, c++20
// twin: tests/memory/pick_owns.c
// Pins: 0037 item 9: without r == p || r == q r is not NULL but nothing relates it to the owned cells, so the write through r has no permission
#include <cerastes.h>
#include <stddef.h>

void set_one(int *p, int *q, int *r)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*q) && r != NULL)
    CERASTES_ENSURES(cerastes_owns(*p) && cerastes_owns(*q))
{
  *r = 1;
}
