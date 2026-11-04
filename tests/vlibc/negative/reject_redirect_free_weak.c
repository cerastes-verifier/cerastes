// expect: rejected cerastes-unsupported_declaration @ L16
// mode: c23, c++20
// twin: tests/vlibc/malloc_explicit.c
// Pins: 0047 item 2 (audit B2) and 0046 item 8: `#pragma weak free` makes free weak, so another definition may replace it: a call is rejected like exit's
// Pre-registered from record 0047 item 2: rejected unsupported_declaration at the call of free; re-registered after audit round 1 (record 0047 item 2): the (D) view call keeps the redirection check
#pragma weak free
#include <stdlib.h>
#include <cerastes.h>

int f(void)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  free(p);  // <- here
  return 0;
}
