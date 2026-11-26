// expect: rejected cerastes-unsupported_declaration @ L24
// expect: rejected cerastes-unsupported_declaration @ L32
// expect: rejected cerastes-unsupported_expression @ L40
// mode: c23, c++20
// twin: tests/memory/array_struct_init.c
// Pins: 0064 items 6 and 12 (audit round 2, issue 6): an element of a braced list that takes the address of (getx(&v[0])) the array being initialized is rejected as 0059 item 3's own-initializer rule has it (probe o6: unsupported_declaration at the declaration, in both modes; the read and the write of the array's own element are in array_struct_reject_more_c.c, because in C++ the elided-braces rule fires first on them and gives unsupported_expression: question 160, report 14); an element initialized from a structure VALUE ({t0, t1}) is rejected with the message that asks for the element's braced list (encode.cpp, item 6); and v[i] = f(), a value stored into an element, is rejected as item 12 says (copy from a structure local: audit round 2, issue 5; edg-backend gives it a message of its own, the tag stays unsupported_expression: to confirm at the first run, question 160 if another) [Flips C: the v[i] = f() and v[i] = (struct P){...} rows are M4.8's (question 333); every row stays where it is]
// Pre-registered from record 0064 items 6 and 12 and audit round 2 (probes o6, st9), before any run: rejected
#include <cerastes.h>

struct P {
  int x;
  int y;
};

int getx(struct P *p)
    CERASTES_REQUIRES(cerastes_reads(*p))
    CERASTES_ENSURES(r, cerastes_reads(*p) && r == p->x);

struct P mk(int a)
    CERASTES_ENSURES(r, r.x == a);

int address_of_itself(void)
{
  struct P v[2] = {{1, 2}, {getx(&v[0]), 3}};  // <- here
  return v[1].x;
}

int from_values(void)
{
  struct P t0 = {1, 2};
  struct P t1 = {3, 4};
  struct P v[2] = {t0, t1};  // <- here
  return v[1].x;
}

int value_into_element(int i)
    CERASTES_REQUIRES(0 <= i && i < 2)
{
  struct P v[2] = {{1, 2}, {3, 4}};
  v[i] = mk(7);  // <- here
  return v[0].x;
}
