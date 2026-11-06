// expect: error uninitialized @ L16
// mode: c23, c++20
// twin: tests/memory/ref_to_uninit_local.c
// Pins: 0052 item 4: the marker changes no verdict: the read before any write of malloc's cell is uninitialized with or without it
// Pre-registered from record 0052 item 4: uninitialized at the read
#include <cerastes.h>
#include <stdlib.h>

int fill(void)
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
