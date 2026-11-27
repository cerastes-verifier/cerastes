// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 13 and 1 (Part E; 0024 item 6): sizeof of a matrix and of a row of it (sizeof(m), sizeof(m[0])), of the types int[2][3] and struct P[3] (the staging pin array_reject_struct_sizeof_type.c and reject_sizeof_2d_array.c are these operands), of a file-scope typedef of a row, and a static_assert over a type operand: every bound is read as written, the sizes follow the LP64 targets (int is 4 bytes)
// Pre-registered from record 0064 items 1 and 13 (Part E), before any run: verifies
#include <cerastes.h>

struct P {
  int x;
  int y;
};

typedef int row[3];

static_assert(sizeof(row) == 12, "a row of three ints");
static_assert(sizeof(int[2][3]) == 24, "two rows");

int sizes(void)
    CERASTES_ENSURES(r, r == 24 + 12 + 24 + 24 + 12)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  return (int)sizeof(m) + (int)sizeof(m[0]) + (int)sizeof(int[2][3]) + (int)sizeof(struct P[3]) + (int)sizeof(row) + (m[0][0] - 1);
}
