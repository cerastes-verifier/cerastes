// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 12 (question 353, as amended on 2026-11-26): exactly 7 counted cells is accepted for a by-value parameter and a by-value result (struct V7 { int m[1][7]; }); split from the first draft so that each file is under the 60 s line: the compound literal, the whole-structure copy and the copy-versus-value count of struct T are in matrix_value_limit_seven_copy.c; negative/matrix_value_limit_eight.c is the rejected side (8 counted cells)
// Pre-registered from record 0064 item 12 (Part E), before any run (test-mutator-11, 2026-11-27: the first draft's param7, result7 and use, the file slimmed after its 72 s and 74 s single probes): verifies (time it first: rule 3's 60 s line)
#include <cerastes.h>

struct V7 {
  int m[1][7];
};

int param7(struct V7 v)
    CERASTES_ENSURES(r, r == 0)
{
  return v.m[0][6] * 0;
}

struct V7 result7(void)
{
  struct V7 v = {{{1, 2, 3, 4, 5, 6, 7}}};
  return v;
}

int use(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct V7 v = {{{1, 2, 3, 4, 5, 6, 7}}};
  struct V7 w = result7();
  return param7(v) + w.m[0][0] * 0;
}
