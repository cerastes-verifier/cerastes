// expect: rejected cerastes-unsupported_expression @ L39
// expect: rejected cerastes-unsupported_expression @ L45
// expect: rejected cerastes-unsupported_expression @ L51
// expect: rejected cerastes-unsupported_expression @ L57
// expect: rejected cerastes-unsupported_expression @ L63
// expect: rejected cerastes-unsupported_expression @ L69
// expect: rejected cerastes-unsupported_expression @ L75
// expect: rejected cerastes-unsupported_expression @ L81
// expect: rejected cerastes-unsupported_expression @ L87
// expect: rejected cerastes-unsupported_expression @ L93
// mode: c23
// c-only: nullptr is C23 and a lone zero that elides braces is C's universal zero initializer; in C++ item 16's other rule (clang++ -Wmissing-braces) rejects these shapes for a different reason
// twin: tests/memory/struct_init_literal_zero_c.c
// Pins: 0064 item 16 (question 325, built by question 339; P84 (a)): in C, a lone zero that is the whole initializer and elides the braces of the first element (its parent an array, or a structure of two or more members) is rejected when it is not an integer literal: '\0', an enumerator, 1 - 1, false, NULL and nullptr (on pointer leaves), -0, +0 and (int)0, one function each, and '\0' once more over a structure whose first member is a structure (the second parent). clang's C warns -Wmissing-braces at -Wall on every one of them in every position measured (a local array, a global array, a structure of two members, a matrix: build/m4.5e1e2b/s0/zero/results.txt) and GCC on none; the literal spellings (0u, 0L, 0x0, 00, 0b0, 0ULL, a macro that is 0) draw no warning and are accepted (struct_init_literal_zero_c.c). Not told by the IL and still accepted by their value (listed for M8): (0), ((0)), (0u) and a macro whose body is (0), which clang also flags
// Pre-registered from record 0064 item 16 (P84 (a), the middle branch), before any run: one unsupported_expression row per spelling, each at its initializer
#include <cerastes.h>
#include <stdbool.h>
#include <stddef.h>

struct P {
  int x;
  int y;
};

struct Q {
  int *x;
  int *y;
};

struct W {
  struct P p;
  int k;
};

enum E { E_ZERO, E_ONE };

int char_zero(void)
{
  struct P v[2] = {'\0'};  // <- here
  return v[0].x;
}

int enumerator_zero(void)
{
  struct P v[2] = {E_ZERO};  // <- here
  return v[0].x;
}

int difference_zero(void)
{
  struct P v[2] = {1 - 1};  // <- here
  return v[0].x;
}

int false_zero(void)
{
  struct P v[2] = {false};  // <- here
  return v[0].x;
}

int null_zero(void)
{
  struct Q v[2] = {NULL};  // <- here
  return v[0].x == NULL;
}

int nullptr_zero(void)
{
  struct Q v[2] = {nullptr};  // <- here
  return v[0].x == NULL;
}

int negative_zero(void)
{
  struct P v[2] = {-0};  // <- here
  return v[0].x;
}

int positive_zero(void)
{
  struct P v[2] = {+0};  // <- here
  return v[0].x;
}

int cast_zero(void)
{
  struct P v[2] = {(int)0};  // <- here
  return v[0].x;
}

int second_parent(void)
{
  struct W w = {'\0'};  // <- here
  return w.k;
}
