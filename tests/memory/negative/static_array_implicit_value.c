// expect: error postcondition @ L18
// mode: c23, c++20
// twin: tests/memory/static_array_implicit_atom.c
// Pins: 0063 items 7 and 9: the entry value of a static array local is unknown in a function that other calls reach (the initializer is its value in main only): r == 1 is not a fact
// Pre-registered from record 0063 item 9 (Part B), before any run: the one error
#include <cerastes.h>

int next(int i)
    CERASTES_REQUIRES(0 <= i && i < 3)
    CERASTES_ENSURES(r, r == 1)
{
  static int c[3] = {0};
  int v = c[i];
  if (v < 0 || v > 99) {
    v = 0;
  }
  c[i] = v + 1;
  return c[i];  // <- here
}

int main(void)
{
  return next(0) - 1;
}
