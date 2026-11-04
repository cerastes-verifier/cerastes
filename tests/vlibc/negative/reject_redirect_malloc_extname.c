// expect: rejected cerastes-unsupported_declaration @ L12
// mode: c23, c++20
// twin: tests/vlibc/malloc_explicit.c
// Pins: 0047 item 2 (audit B2) and 0046 item 8: `#pragma redefine_extname malloc my_malloc` before <stdlib.h> gives malloc another external name, so a call of it does not reach vlibc's contract (the pragma is not a declaration: the rejection is at the call)
// Pre-registered from record 0047 item 2: rejected unsupported_declaration at the call of malloc; re-registered after audit round 1 (record 0047 item 2): the view call keeps the redirection check
#pragma redefine_extname malloc my_malloc
#include <stdlib.h>
#include <cerastes.h>

int f(void)
{
  int *p = (int *)malloc(sizeof(int));  // <- here
  free(p);
  return 0;
}
