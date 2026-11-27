// expect: error check @ L13
// mode: c++20
// twin: tests/memory/matrix_global_ptr.cpp
// Pins: 0064 item 9 (0063 item 9's escape rule, as negative/global_array_escape_tab.cpp): with external linkage the matrix's values are unknown (another unit's initializer may write *tab[0][0] before main): h is unknown in main
// Pre-registered from record 0064 item 9 (Part E), before any run: the one error
#include <cerastes.h>

static int h = 7;
int *tab[2][2] = {{&h, 0}, {0, 0}};

int main()
{
  CERASTES_CHECK(h == 7);  // <- here
  return 0;
}
