// expect: verified
// expect: native-ok
// mode: c23, c++20
// Port of VeriFast's examples/global_points_to_syntax.c (54 lines; ~/Documents/verifast at 0fb63506) to cerastes (decision 0041, M4.5b2a).
// Adaptation: `integer(&global1, ?val1)` and `global1 |-> ?val1` become `cerastes_owns(global1)`, and `val1 + 5` becomes `global1 == cerastes_old(global1) + 5`: one
// spelling for both functions, which stay as two. VeriFast ran it with -disable_overflow_check; cerastes checks every `+ 5` (nothing wraps, 0022) and every `(char)` conversion,
// so the requires bound the values (`global1 <= INT_MAX - 5`, `global3 <= UINT_MAX - 5`, `global4 <= 122`, the last so that `(char)(global4 + 5)` fits plain char of either signedness);
// `(char)(val4 + 5)` is `global4 == cerastes_old(global4) + 5` (the same value under the bound). main: `open_module()` and the `module` ghost are gone (main receives the
// globals from program_start, 0041 item 4), each `//@ assert` is a CERASTES_CHECK, the final `leak` is dropped (a global's block is never freed).
// Line map (this file : VeriFast's):  14 : 3   20 : 9   27 : 19   32 : 24   34 : 26   41 : 30   48 : 37   50 : 43   51 : 44   52 : 45   53 : 46   54 : 47
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
  CERASTES_CHECK(global1 == 0 && global2 == 20 && global3 == 30 && global4 == 'k' && global5 == 'l');
  return 0;
}
