// expect: error leak @ L21
// mode: c23, c++20
// twin: tests/memory/marked_static_local.c
// Pins: 0054 item 5: the marker is not ownership: the block stored in a non-owning (marked) static local and not freed is a leak at the function's closing brace
// Pre-registered from record 0054 item 5 (only owning variables are exempt; the marker is not an owner): leak at the closing brace
#include <cerastes.h>
#include <stdlib.h>

int f(void)
    CERASTES_ENSURES(r, r == 0 || r == 1)
{
  CERASTES_REF_TO_UNINIT static int *s = NULL;
  s = (int *)malloc(sizeof(int));
  if (s == NULL) {
    return 1;
  }
  *s = 3;
  
  s = NULL;
  return 0;
}  // <- here

int main(void)
{
  return f();
}
