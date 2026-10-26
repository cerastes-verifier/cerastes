// expect: rejected cerastes-unrecognized_attribute @ L9
// mode: c23, c++20
// twin: tests/scalar/if_else.c
// Pins: 0024 item 1 (amends 0011 item 7): the encoder reads a redeclaration's attributes through the source sequence list. The tag is the one for an unknown namespace
// (ak_unrecognized, as reject_unrecognized_attribute.c); it is the redeclaration's line. EDG may print its own warning for the unregistered namespace first: see INDEX
#include <cerastes.h>

int g(int x);
int g(int x) [[unreg_ns::a2(x > 0)]];
int g(int x) { return x; }
