// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 items 4, 5, 11 (audit round 2 (j)): in a program with no allocation and no block site an owning field may be initialized to NULL
// Pre-registered from audit round 2 on Part A (record 0055 as revised, 78838ca), h_value_noheap_ok.c: verifies in both modes
#include <cerastes.h>
#include <stddef.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int main(void)
{
  int x = 1;
  struct buf s = { NULL, 0 };
  return s.n + x;
}
