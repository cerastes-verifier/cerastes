// expect: rejected cerastes-unrecognized_attribute @ L8
// mode: c23, c++20
// twin: tests/scalar/if_else.c
// Pins: 0011 item 7: a misspelt cerastes attribute is an error, not EDG's warning
// Assumption A5 (INDEX.md): the tag is unrecognized_attribute (0011 item 7).
#include <cerastes.h>

[[ceraste::requires(x > 0)]] int ident(int x) { return x; }
