// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 2 and 3 (Part E; coordinator's ruling on the cell types): the cell of a local matrix may be bool (a flag cell whose value is its own range) or a pointer to an object type (the cell's value is WF, the pointee keeps its own block): b[i][j] is written and read at symbolic indexes, and p[i][j] holds the address of x and is dereferenced
// Pre-registered from record 0064 items 2 and 3 (Part E), before any run: verifies
#include <cerastes.h>

int bools(int i, int j)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 2)
    CERASTES_ENSURES(r, r == 1)
{
  bool b[2][2] = {{false, false}, {false, false}};
  b[i][j] = true;
  return b[i][j] ? 1 : 0;
}

int ptrs(int i, int j)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 2)
    CERASTES_ENSURES(r, r == 5)
{
  int x = 5;
  int *p[2][2] = {{0, 0}, {0, 0}};
  p[i][j] = &x;
  return *p[i][j];
}
