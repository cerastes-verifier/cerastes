// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0024 item 5 (amends 0022 items 2-3): a character literal whose value fits its type is the number written; the edge of each
// literal kind ('\x7f', L'\x7fffffff', u'\xffff', U'\xffffffff') converts exactly. Each twin is this file with one literal past an edge.
#include <cerastes.h>

int letter(void)
    CERASTES_ENSURES(r, r == 97)
{
  return 'a';
}

int hex_max(void)
    CERASTES_ENSURES(r, r == 127)
{
  return '\x7f';
}

int octal_max(void)
    CERASTES_ENSURES(r, r == 127)
{
  return '\177';
}

int nul(void)
    CERASTES_ENSURES(r, r == 0)
{
  return '\0';
}

int wide_max(void)
    CERASTES_ENSURES(r, r == 2147483647)
{
  return (int)L'\x7fffffff';
}

int utf16_max(void)
    CERASTES_ENSURES(r, r == 65535)
{
  return (int)u'\xffff';
}

unsigned utf32_max(void)
    CERASTES_ENSURES(r, r == 4294967295u)
{
  return (unsigned)U'\xffffffff';
}

char into_char(void)
    CERASTES_ENSURES(r, r == 127)
{
  char c = '\x7f';
  return c;
}

int alphabet_span(void)
    CERASTES_ENSURES(r, r == 25)
{
  return 'z' - 'a';
}

int widened(signed char c)
    CERASTES_REQUIRES(c >= 0)
    CERASTES_ENSURES(r, r == c + 128 && r >= 128)
{
  return c + 128;
}

int edge_sum(void)
    CERASTES_ENSURES(r, r == 255)
{
  return '\x7f' + 128;
}
