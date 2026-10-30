// expect: error check @ L48
// mode: c23, c++20
// twin: tests/memory/verifast/global_points_to_syntax.c
// Pins: 0041 item 8 and the port of VeriFast's global_points_to_syntax: after both calls global3 is 30, not 31 (main's third check)
#include <limits.h>
#include <cerastes.h>

static int global1 = -10;
static int global2 = 10;
static unsigned int global3 = 20;
static char global4 = 'a';
static char global5 = 'b';

void modify_primitive_globals_old_syntax(void)
    CERASTES_REQUIRES(cerastes_owns(global1) && cerastes_owns(global2) && cerastes_owns(global3) && cerastes_owns(global4) && cerastes_owns(global5) &&
                      global1 <= INT_MAX - 5 && global2 <= INT_MAX - 5 && global3 <= UINT_MAX - 5 && global4 <= 122 && global5 <= 122)
    CERASTES_ENSURES(cerastes_owns(global1) && cerastes_owns(global2) && cerastes_owns(global3) && cerastes_owns(global4) && cerastes_owns(global5) &&
                     global1 == cerastes_old(global1) + 5 && global2 == cerastes_old(global2) + 5 && global3 == cerastes_old(global3) + 5 &&
                     global4 == cerastes_old(global4) + 5 && global5 == cerastes_old(global5) + 5)
{
  global1 = global1 + 5;
  global2 = global2 + 5;
  global3 = global3 + 5;
  global4 = (char)(global4 + 5);
  global5 = (char)(global5 + 5);
}

void modify_primitive_globals_new_syntax(void)
    CERASTES_REQUIRES(cerastes_owns(global1) && cerastes_owns(global2) && cerastes_owns(global3) && cerastes_owns(global4) && cerastes_owns(global5) &&
                      global1 <= INT_MAX - 5 && global2 <= INT_MAX - 5 && global3 <= UINT_MAX - 5 && global4 <= 122 && global5 <= 122)
    CERASTES_ENSURES(cerastes_owns(global1) && cerastes_owns(global2) && cerastes_owns(global3) && cerastes_owns(global4) && cerastes_owns(global5) &&
                     global1 == cerastes_old(global1) + 5 && global2 == cerastes_old(global2) + 5 && global3 == cerastes_old(global3) + 5 &&
                     global4 == cerastes_old(global4) + 5 && global5 == cerastes_old(global5) + 5)
{
  global1 = global1 + 5;
  global2 = global2 + 5;
  global3 = global3 + 5;
  global4 = (char)(global4 + 5);
  global5 = (char)(global5 + 5);
}

int main(void)
{
  CERASTES_CHECK(global1 == -10 && global2 == 10 && global3 == 20 && global4 == 'a' && global5 == 'b');
  modify_primitive_globals_old_syntax();
  CERASTES_CHECK(global1 == -5 && global2 == 15 && global3 == 25 && global4 == 'f' && global5 == 'g');
  modify_primitive_globals_new_syntax();
  CERASTES_CHECK(global1 == 0 && global2 == 20 && global3 == 31 && global4 == 'k' && global5 == 'l');
  return 0;
}
