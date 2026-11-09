// expect: error uninitialized @ L17
// mode: c23, c++20
// twin: tests/memory/marked_global.c
// Pins: 0054 item 5 (default mode): the marker changes no Viper, the cells of malloc's block still carry their flags: a read before the write is uninitialized
// Pre-registered from record 0054 item 5 and 0019/0042: uninitialized at the read
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
