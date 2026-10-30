// expect: error lifetime @ L16
// mode: c23, c++20
// twin: tests/memory/static_local_return_address.c
// Pins: 0041 item 12 (audit S5, a documented false alarm, lifted by M4.5d2): the documented false alarm until M4.5d2: the caller knows only WF(r) of where()'s result (no atom on *r and no r == &s before owning results), so the copy `int *p = where();` is the first use of an unframed pointer result and fails lifetime (0028 item 3a)
#include <cerastes.h>
#include <stddef.h>

int *where(void)
{
  static int s = 5;
  return &s;
}

int main(void)
{
  int *p = where();
  if (p == NULL) {
    return 1;
  }
  return 0;
}
