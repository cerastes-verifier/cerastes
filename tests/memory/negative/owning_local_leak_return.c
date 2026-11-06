// expect: error leak @ L18
// mode: c23, c++20
// twin: tests/memory/owning_local_free.c
// Pins: 0052 item 3: the owning local's block still held at a return: leak at the return statement (without owning the same program leaks at the function's closing brace, heap_leak_no_free.c)
// Pre-registered from record 0052 item 3: leak at the return statement
#include <cerastes.h>
#include <stdlib.h>

int use_and_forget(void)
    CERASTES_ENSURES(r, r == 7 || r == -1)
{
  CERASTES_OWNING int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  *p = 7;
  int v = *p;
  return v;  // <- here
}
