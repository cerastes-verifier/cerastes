// expect: rejected cerastes-unsequenced_calls @ L21
// expect: rejected cerastes-unsequenced_calls @ L28
// expect: rejected cerastes-unsequenced_calls @ L34
// expect: rejected cerastes-unsequenced_calls @ L41
// c-only: compound literals are C (C++ rejects them, record 0044 item 7)
// twin: tests/memory/compound_literal_effects_allowed.c
// Pins: 0063 item 12 and 0044 item 7 (audit round 2, B1', question 308; probes q64, q65, q75, q73): a writing compound literal that is one element of a declaration's braced list, beside another element: the elements of a list are indeterminately sequenced (C23 6.7.11p24; clang warns -Wunsequenced by default), so the list rule `two elements of an initializer list, one of them with a side effect` must see inside the literal: array form, structure form, over an element of a global array, and the scalar form &(int){x++}. One function per form keeps the four lines independent. The tag is `unsequenced_calls` (the coordinator's message said unsupported_calls; diagnostics.def has no such tag) and the lines are to confirm at the run
// Pre-registered from audit round 2 of record 0063 (B1'), before any run: rejected
#include <cerastes.h>

struct P {
  int *p;
  int *q;
};

static int g[1] = {1};

int array_form(void)
{
  int x = 1;
  int *a[2] = {(int[]){x++}, (int[]){x}};  // <- here
  return a[1][0] - 2;
}

int struct_form(void)
{
  int x = 1;
  struct P s = {(int[]){x++}, (int[]){x}};  // <- here
  return s.q[0] - 2;
}

int global_form(void)
{
  int *a[2] = {(int[]){g[0]++}, (int[]){g[0]}};  // <- here
  return a[1][0] - 2;
}

int scalar_form(void)
{
  int x = 1;
  int *a[2] = {&(int){x++}, &(int){x}};  // <- here
  return *a[1] - 2;
}
