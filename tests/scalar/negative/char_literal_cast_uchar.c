// expect: error conversion @ L11
// mode: c23, c++20
// twin: tests/scalar/char_literal.c
// Pins: 0024 item 5: char_literal.c with letter's literal inside (unsigned char)'\xff': the cast comes after the literal's own conversion, which is the error
// Rewrite: write 255 or 0xff
#include <cerastes.h>

int letter(void)
    CERASTES_ENSURES(r, r == 97)
{
  return (unsigned char)'\xff';
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

int widened(char c)
    CERASTES_REQUIRES(c >= 0)
    CERASTES_ENSURES(r, r == c + 128)
{
  return c + 128;
}

int widened_edge(void)
    CERASTES_ENSURES(r, r == 255)
{
  return widened('\x7f');
}
