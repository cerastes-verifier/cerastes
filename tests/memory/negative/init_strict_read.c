// expect: error uninitialized @ L14
// mode: c23, c++20
// twin: tests/memory/init_strict_ok.c
// verify-options: --strict-init
// Pins: 0042 item 8: strict mode adds rejections and removes none: the path-precise 0019 error is still reported for a marked local
#include <cerastes.h>

int use(int c)
{
  CERASTES_UNINIT int x;
  if (c != 0) {
    x = 1;
  }
  return x;  // <- here
}
