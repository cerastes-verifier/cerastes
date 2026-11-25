// expect: rejected cerastes-unsupported_expression @ L23
// expect: rejected cerastes-unsupported_expression @ L30
// expect: rejected cerastes-unsupported_expression @ L37
// expect: rejected cerastes-unsupported_expression @ L44
// expect: rejected cerastes-unsupported_expression @ L51
// expect: rejected cerastes-unsupported_expression @ L57
// c-only: compound literals are C (C++ rejects them, record 0044 item 7)
// twin: tests/memory/compound_literal_effects_allowed.c
// Pins: 0063 item 12 and 0044 item 7 (audit round 2, B1, question 308): a literal's initializers are not full expressions and the encoder's comma is no sequence point, so an element that writes (++, --, compound assignment, assignment, among a call's arguments), in a literal that is neither the whole full expression nor a declaration's initializer, is unsequenced with the store to the same object: rejected at the literal (clang warns -Wunsequenced on the first by default); the sixth line is an element of a global array (p52g). Every line is to confirm at the run (the tag is the coordinator's ruling, not a run)
// Pre-registered from audit round 2 of record 0063 (B1, question 308), before any run: rejected
#include <cerastes.h>

static int g[2] = {1, 0};

int id(int v)
{
  return v;
}

int increment(void)
{
  int x = 1;
  x = (int[]){x++}[0];  // <- here
  return x - 1;
}

int decrement(void)
{
  int x = 1;
  x = (int[]){x--}[0];  // <- here
  return x - 1;
}

int compound(void)
{
  int x = 1;
  x = (int[]){x += 2}[0];  // <- here
  return x - 1;
}

int assigned(void)
{
  int x = 1;
  x = (int[]){(x = 3)}[0];  // <- here
  return x - 1;
}

int argument(void)
{
  int x = 1;
  x = (int[]){id(x++)}[0];  // <- here
  return x - 1;
}

int global(void)
{
  g[0] = (int[]){g[0]++}[0];  // <- here
  return g[0];
}
