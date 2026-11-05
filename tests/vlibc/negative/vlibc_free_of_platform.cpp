// expect: error invalid-free @ L15
// twin: tests/vlibc/malloc_cstdlib.cpp
// Pins: 0050 items 3, 5, 7 (0048 item 4): vlibc_free stays accepted in C++, and a block it meets there is the platform's: invalid-free at the call
// Pre-registered from record 0050 item 7: error invalid-free at the vlibc_free call (one error)
#include <vlibc/stdlib.h>
#include <cerastes.h>

int f()
{
  int *p = static_cast<int *>(malloc(sizeof(int)));
  if (p == nullptr) {
    return -1;
  }
  *p = 2;
  vlibc_free(p);  // <- here
  return 0;
}
