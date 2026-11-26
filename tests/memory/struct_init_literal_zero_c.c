// expect: verified
// expect: native-ok
// mode: c23
// c-only: a lone zero that elides the braces of an array's first structure is C's universal zero initializer; in C++ item 16's other rule rejects the shape (clang++ -Wmissing-braces)
// Pins: 0064 item 16 (question 325, 339): the literal spellings of a lone zero stay accepted where the non-literal ones are rejected (negative/struct_init_reject_nonliteral_zero_c.c): 0u, 0L, 0x0, 00, 0b0, 0ULL and a macro that is 0, over an array of structures. clang's C and GCC are silent on each at -Wall -Wextra (build/m4.5e1e2b/s0/zero/results.txt). Every leaf is zero. (0) and its relatives are accepted too but clang flags them, so they cannot be in a native-ok test: listed for M8
// Pre-registered from record 0064 item 16 (P84 (a)), before any run: verifies
#include <cerastes.h>

#define ZERO 0

struct P {
  int x;
  int y;
};

int unsigned_zero(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct P v[2] = {0u};
  return v[1].y;
}

int long_zero(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct P v[2] = {0L};
  return v[0].x;
}

int hex_zero(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct P v[2] = {0x0};
  return v[1].x;
}

int octal_zero(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct P v[2] = {00};
  return v[0].y;
}

int binary_zero(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct P v[2] = {0b0};
  return v[1].y;
}

int long_long_zero(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct P v[2] = {0ULL};
  return v[0].x;
}

int macro_zero(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct P v[2] = {ZERO};
  return v[1].x;
}
