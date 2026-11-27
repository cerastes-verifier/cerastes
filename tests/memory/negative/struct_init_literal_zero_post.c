// expect: error postcondition @ L20
// mode: c23
// twin: tests/memory/struct_init_literal_zero_c.c
// c-only: a lone zero that elides the braces of an array's first structure is C's universal zero initializer; in C++ item 16's other rule rejects the shape (clang++ -Wmissing-braces)
// Pins: 0064 item 16: unsigned_zero promises 1 where the zero-filled element holds 0; the other functions are the positive's
// Pre-registered from record 0064 item 16 (P84 (a)), before any run: postcondition at unsigned_zero's return, one error
#include <cerastes.h>

#define ZERO 0

struct P {
  int x;
  int y;
};

int unsigned_zero(void)
    CERASTES_ENSURES(r, r == 1)
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
