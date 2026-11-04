// expect: error invalid-free @ L16
// twin: tests/vlibc/malloc_cstdlib.cpp
// Pins: 0047 item 6 and 0046 item 9: a twin of free's trusted requires through <cstdlib>: the second free finds no token
// Pre-registered from record 0047 item 6: invalid-free at the second std::free
#include <cstdlib>
#include <cerastes.h>

int roundtrip()
{
  int *p = static_cast<int *>(std::malloc(sizeof(int)));
  if (p == nullptr) {
    return -1;
  }
  *p = 7;
  std::free(p);
  std::free(p);  // <- here
  return 7;
}
