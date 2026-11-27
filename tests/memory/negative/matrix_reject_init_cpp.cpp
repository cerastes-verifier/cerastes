// expect: rejected cerastes-unsupported_expression @ L11
// expect: rejected cerastes-unsupported_expression @ L17
// mode: c++20
// twin: tests/memory/matrix_init.c
// Pins: 0064 item 16 (question 304, 321): in C++ a list that elides the braces of the rows is rejected, and so is {0} over a matrix, whose 0 elides the braces of the first row, an element of an array (clang++ flags both at -Wall: -Wmissing-braces); the C row of the same {0} is accepted (matrix_init.c)
// Pre-registered from record 0064 item 16 (Part E), before any run: rejected (tag as array_struct_reject_zero_cpp.cpp, to confirm at the joined run)
#include <cerastes.h>

int elided()
{
  int m[2][3] = {1, 2, 3, 4, 5, 6};  // <- here
  return m[1][2];
}

int zero()
{
  int m[2][3] = {0};  // <- here
  return m[1][2];
}
