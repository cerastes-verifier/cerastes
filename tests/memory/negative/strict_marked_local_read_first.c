// expect: error uninitialized @ L17
// mode: c23, c++20
// verify-options: --strict-init
// twin: tests/memory/strict_marked_local_alloc.c
// Pins: 0052 item 4: under --strict-init a marked local still obeys the initialization rule: reading the cell before writing it is uninitialized
// Pre-registered from record 0052 item 4: uninitialized at the read
#include <cerastes.h>
#include <stdlib.h>

int via_init(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  CERASTES_REF_TO_UNINIT int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  int v = *p;  // <- here
  *p = v;
  free(p);
  return 0;
}
