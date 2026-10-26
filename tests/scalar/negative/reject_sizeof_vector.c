// expect: rejected cerastes-unsupported_type @ L11
// mode: c23, c++20
// twin: tests/scalar/sizeof_value.c
// Pins: 0024 item 6 (cut 3 candidate): a vector type in a type operand
#include <cerastes.h>

typedef int v4si __attribute__((vector_size(16)));

int vec(void)
{
  return (int)sizeof(v4si);
}
