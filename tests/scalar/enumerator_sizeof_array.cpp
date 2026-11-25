// expect: verified
// expect: native-ok
// Pins: 0063 item 14 (flip A of M4.5e1e1, was reject_enumerator_sizeof_array.cpp): an enumerator initialized by sizeof of an array type: 4; unchanged, line for line (twin negative/enumerator_sizeof_array_wrap.cpp)
#include <cerastes.h>

enum {
  SZ = sizeof(char[4])
};

int size(void)
{
  return SZ;
}
