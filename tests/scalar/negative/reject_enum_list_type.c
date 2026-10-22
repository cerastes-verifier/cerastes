// expect: rejected cerastes-unsupported_expression @ L9
// c-only: in C++ the program verifies with C++'s value (enum_list_type.cpp)
// twin: tests/scalar/enum_list_type.cpp
// Pins: 0018 item 10: C23 types U as int inside the list (6.7.3.3) and C++ as unsigned int, so U - 3 > 0 differs (0 in C, 1 in C++): rejected in C
#include <cerastes.h>

enum {
  U = 2u,
  V = U - 3 > 0
};

int v_value(void)
{
  return V;
}
