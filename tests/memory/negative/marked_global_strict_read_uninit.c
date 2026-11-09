// expect: error uninitialized @ L18
// mode: c23, c++20
// verify-options: --strict-init
// twin: tests/memory/marked_global_strict.c
// Pins: 0054 item 5: --strict-init rejects shapes, it does not change what the flags prove: the read before the write is still uninitialized
// Pre-registered from record 0054 item 5: uninitialized at the read
#include <cerastes.h>
#include <stdlib.h>

CERASTES_REF_TO_UNINIT int *g = NULL;

int main(void)
{
  g = (int *)malloc(sizeof(int));
  if (g == NULL) {
    return 1;
  }
  int v = *g;  // <- here
  free(g);
  g = NULL;
  return v;
}
