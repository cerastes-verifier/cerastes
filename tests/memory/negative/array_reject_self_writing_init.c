// expect: rejected cerastes-unsupported_declaration @ L19
// expect: rejected cerastes-unsupported_declaration @ L25
// mode: c23, c++20
// twin: tests/memory/array_init_dynamic.c
// Pins: 0059 item 3 (audit round 1, item 5): an element of the initializer that writes the array being declared, or gives its address to a call, is rejected like one that reads it (0043 item 6's rule for structures), at the declaration (not lifted)
// Pre-registered from record 0059 item 3 as updated by audit round 1
#include <cerastes.h>

int g(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  *p = 3;
  return 1;
}

int writes(void)
{
  int a[2] = {1, (a[0] = 6)};
  return a[1];
}

int calls(void)
{
  int b[2] = {1, g(&b[0])};
  return b[1];
}
