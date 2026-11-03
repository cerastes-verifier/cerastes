// expect: rejected cerastes-unsupported_declaration @ L7
// mode: c23, c++20
// twin: tests/memory/struct_local.c
// Pins: 0043 item 2: a packed struct (a member's address may be misaligned) is rejected (M8)
#include <cerastes.h>

struct __attribute__((packed)) P {  // <- here
  char c;
  int i;
};

int ident(int a)
{
  return a;
}
