// expect: error check @ L17
// mode: c23, c++20
// twin: tests/memory/array_rows_struct_elem_global.c
// Pins: 0064 item 9: g[1].a[3] is 9 (the listed value of the second element's fourth cell), so a check that it is 8 fails
// Pre-registered from record 0064 item 9 (Part E), before any run: the one error; corrected once after the first run on j4a (question 160): the front end's #550 warning on `g` (read only by the check) made the run unclean, so main returns `g[0].a[0] - 1` (program only, the expect line stands)
#include <cerastes.h>

struct W {
  int a[4];
  int n;
};

static struct W g[2] = {{{1, 2, 3, 4}, 5}, {{6, 7, 8, 9}, 10}};

int main(void)
{
  CERASTES_CHECK(g[1].a[3] == 8);  // <- here
  return g[0].a[0] - 1;
}
