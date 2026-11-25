// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0063 item 14 (flip A of M4.5e1e1, the program unchanged, line for line, was reject_typedef_array_used.c, both of its lines): a typedef of an array type and its use as a type operand, sizeof(buf_t) == 4 (twin negative/typedef_array_used_wrong.c)
// Pre-registered from record 0063 item 14 (Part D), before any run: verifies
#include <cerastes.h>

typedef char buf_t[4];

int used(void)
{
  return (int)sizeof(buf_t);
}
