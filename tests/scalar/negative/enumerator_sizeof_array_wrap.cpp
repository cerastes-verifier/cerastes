// expect: error overflow @ L8
// twin: tests/scalar/enumerator_sizeof_array.cpp
// Pins: 0063 item 14: the same rule in a C++ enumerator: the bound of char[65536u * 65536u + 1] wraps; overflow at the bound
// Pre-registered from record 0063 item 14 (Part D), before any run: the one error
#include <cerastes.h>

enum {
  SZ = sizeof(char[65536u * 65536u + 1])  // <- here
};

int size(void)
{
  return SZ;
}
