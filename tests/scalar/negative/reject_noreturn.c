// expect: rejected cerastes-unsupported_attribute @ L7
// mode: c23, c++20
// twin: tests/scalar/if_else.c
// Pins: 0011 item 7: a function declared [[noreturn]] is rejected
#include <cerastes.h>

[[noreturn]] void stop(void);
