// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 12 (question 353, as amended on 2026-11-26): exactly 7 counted cells is accepted for a compound literal assigned to a structure and for a whole-structure copy (struct V7 { int m[1][7]; }); the value half of the 7-cell pair is matrix_value_limit_seven.c, and the copy-versus-value count of struct T is matrix_value_limit_copy_t.c
// Pre-registered from record 0064 item 12 (Part E), before any run (test-mutator-11, 2026-11-27: the first draft's literal7 (now the ASSIGNMENT form, which is a value in both modes: in C++ `struct V7 v = (struct V7){...}` is a braced initialization and builds no value) and copy7, the file slimmed after the first draft's 72 s and 74 s single probes and split again after a second draft that held copy_t too took 64 s and 65 s at load 10 to 17): verifies (time it first: rule 3's 60 s line); copy7 gained the check b.m[0][6] == 7 (test-mutator-11, before its re-probe), the right-value sibling of negative/matrix_value_limit_seven_wrong.c's check (question 388)
#include <cerastes.h>

struct V7 {
  int m[1][7];
};

int literal7(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct V7 v = {{{0, 0, 0, 0, 0, 0, 0}}};
  v = (struct V7){{{1, 2, 3, 4, 5, 6, 7}}};
  return v.m[0][0] * 0;
}

int copy7(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct V7 a = {{{1, 2, 3, 4, 5, 6, 7}}};
  struct V7 b = {{{0, 0, 0, 0, 0, 0, 0}}};
  b = a;
  CERASTES_CHECK(b.m[0][6] == 7);
  return b.m[0][6] * 0;
}
