// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0063 items 3 and 9: array globals read and written by index in main: a full list, a partial list with the zero rest, no initializer with internal linkage (zero), bool, enumeration and pointer elements (a pointer to another global and null), sizeof(g) / sizeof(g[0]), a write and a read at a variable index
// Pre-registered from record 0063 item 9 (Part B), before any run: verifies
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

static int full[3] = {1, 2, 3};
static int part[4] = {5};
static int zero[2];
static bool flags[2] = {true};
static enum color col[2] = {BLUE, GREEN};
static int x = 7;
static int *ptrs[2] = {&x, 0};

int main(void)
{
  CERASTES_CHECK(full[0] == 1 && full[2] == 3);
  CERASTES_CHECK(part[0] == 5 && part[1] == 0 && part[3] == 0);
  CERASTES_CHECK(zero[0] == 0 && zero[1] == 0);
  CERASTES_CHECK(sizeof(full) / sizeof(full[0]) == 3);
  CERASTES_CHECK(flags[0] && !flags[1]);
  CERASTES_CHECK(col[0] == BLUE && col[1] == GREEN);
  CERASTES_CHECK(ptrs[0] == &x && ptrs[1] == 0);
  full[1] = 20;
  part[2] = full[1] + part[0];
  CERASTES_CHECK(part[2] == 25 && full[1] == 20);
  int i = 2;
  CERASTES_CHECK(full[i] == 3);
  return zero[0] + (flags[0] ? 1 : 0) + (int)col[1] + (ptrs[1] == 0 ? 0 : 1) - 2 + (RED - RED) + (full[i] - 3);
}
