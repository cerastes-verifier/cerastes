// expect: rejected cerastes-unsupported_declaration @ L10
// expect: rejected cerastes-unsupported_declaration @ L11
// mode: c23, c++20
// twin: tests/memory/global_array_declared.c
// Pins: 0063 item 9 (audit round 2, S2; S7 (c)): the decay is recognized by its expr, not by offset 0 and an empty subobject path alone: g + 0 and (int *)&g are folded addresses of g at offset 0 whose expr is not array_to_pointer of the variable, so they are rejected like &g[k] (the auditor's p07 and p07b: rejected unsupported_declaration in both modes)
// Pre-registered from audit round 2 of record 0063 (S2), before any run: rejected
#include <cerastes.h>

static int g[3];
int *plus0 = g + 0;  // <- here
int *cast = (int *)&g;  // <- here

int ident(int a)
{
  return a;
}
