// expect: error postcondition @ L12
// mode: c23, c++20
// twin: tests/memory/matrix_init.c
// Pins: 0064 item 6: the zero fill and the listed values of a partial list are exact: the six cells of {{1}, {4, 5}} sum to 10, so a postcondition of 11 fails at the return (the zero-filled cells are known to be zero, not merely initialized)
// Pre-registered from record 0064 item 6 (Part E), before any run: the one error
#include <cerastes.h>

int partial(void)
    CERASTES_ENSURES(r, r == 11)
{
  int m[2][3] = {{1}, {4, 5}};
  return m[0][0] + m[0][1] + m[0][2] + m[1][0] + m[1][1] + m[1][2];  // <- here
}
