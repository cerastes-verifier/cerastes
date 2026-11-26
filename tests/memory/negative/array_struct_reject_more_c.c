// expect: rejected cerastes-unsupported_declaration @ L24
// expect: rejected cerastes-unsupported_declaration @ L30
// expect: rejected cerastes-unsupported_expression @ L38
// expect: rejected cerastes-unsequenced_calls @ L45
// expect: rejected cerastes-unsequenced_calls @ L52
// mode: c23
// c-only: a compound literal as a statement value is C (C++ has no (struct P){...}); and in C++ the elements of one braced list are evaluated left to right (verifies), so the unsequenced_calls rows are C's, and the own-initializer rows of the read and the write are C's by tag
// twin: tests/memory/array_struct_init.c
// Pins: 0064 items 6 and 12 (audit round 2, issue 6): an element of a braced list that reads (v[0].x) or writes ((v[0].x = 5)) the array being initialized is rejected as 0059 item 3's own-initializer rule has it (probes o1, o11: unsupported_declaration in C; in C++ the same lines are rejected by the elided-braces rule, unsupported_expression, so they are rows of the C file); v[i] = (struct P){1, 2}, a compound literal stored into an element, is rejected as v[i] = f() is (probe st1; unsupported_expression); two elements of one C braced list with calls ({{inc(&c)}, {inc(&c)}}, probe o9) or with side effects ({{c++, c++}, {c++}}, probe o10) are rejected unsequenced_calls at the declaration, C's initializer-list elements being indeterminately sequenced. All three tags are to confirm at the first run [Flips C: the v[i] = f() and v[i] = (struct P){...} rows are M4.8's (question 333); every row stays where it is]
// Pre-registered from record 0064 items 6 and 12 and audit round 2 (probes o1, o11, st1, o9, o10), before any run: rejected
#include <cerastes.h>

struct P {
  int x;
  int y;
};

int inc(int *c)
    CERASTES_REQUIRES(cerastes_owns(*c) && 0 <= *c && *c < 100)
    CERASTES_ENSURES(r, cerastes_owns(*c) && *c == cerastes_old(*c) + 1 && r == *c);

int reads_itself(void)
{
  struct P v[2] = {{1, 2}, {v[0].x, 3}};  // <- here
  return v[1].x;
}

int writes_itself(void)
{
  struct P v[2] = {{1, 2}, {(v[0].x = 5), 3}};  // <- here
  return v[1].x;
}

int literal_into_element(int i)
    CERASTES_REQUIRES(0 <= i && i < 2)
{
  struct P v[2] = {{1, 2}, {3, 4}};
  v[i] = (struct P){1, 2};  // <- here
  return v[0].x;
}

int calls_in_one_list(void)
{
  int c = 0;
  struct P v[2] = {{inc(&c)}, {inc(&c)}};  // <- here
  return v[0].x;
}

int increments_in_one_list(void)
{
  int c = 0;
  struct P v[2] = {{c++, c++}, {c++}};  // <- here
  return v[0].x;
}
