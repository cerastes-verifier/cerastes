// expect: error invalid-free @ L16
// mode: c23, c++20
// twin: tests/vlibc/vlibc_alloc_roundtrip.c
// Pins: 0050 item 4 and audit round 1 B1: the program's only vlibc call is vlibc_free (no vlibc allocation anywhere, through the shim's <stdlib.h>), and it frees a block of the platform's malloc: the value row of vlibc_free is emitted whatever the program allocates, in C and in C++, so the program is invalid-free and not an internal check
// Pre-registered from record 0050 item 4 (audit round 1, B1): invalid-free at the vlibc_free call, one error, in both modes
#include <stdlib.h>
#include <cerastes.h>

int f(void)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  *p = 2;
  vlibc_free(p);  // <- here
  return 0;
}
