// expect: error check @ L17
// mode: c23, c++20
// twin: tests/memory/global_array_struct_declared.c
// Pins: 0064 item 9 (0041 item 3, per leaf): a global array of structures with external linkage and no initializer has unknown elements in main (a C tentative definition may merge with another unit's, a C++ external one may be written by another unit's initializer): u[0].x == 0 is not a fact
// Pre-registered from record 0064 item 9 (Part C2), before any run: the one error
#include <cerastes.h>

struct P {
  int x;
  int y;
};

struct P u[3];

int main(void)
{
  CERASTES_CHECK(u[0].x == 0);  // <- here
  return 0;
}
