// expect: error overflow @ L10
// mode: c23, c++20
// twin: tests/memory/matrix_sizeof.c
// Pins: 0064 item 13 (0063 item 14, 0024): a bound inside a computed type is read as written and checked as written, per dimension: 65536u * 65536u + 1 overflows unsigned int (the front end wraps it to one element), so the inner bound of the type operand is an overflow at the operand, as tests/scalar/negative/reject_sizeof_array_huge.c has it for a one-dimensional type (error overflow at the line of the sizeof)
// Pre-registered from record 0064 item 13 (Part E), before any run: the one error, at the sizeof's line (by the precedent reject_sizeof_array_huge.c; the matrix's inner dimension is the new part)
#include <cerastes.h>

int sizes(void)
{
  return (int)sizeof(int[2][65536u * 65536u + 1]);  // <- here
}
