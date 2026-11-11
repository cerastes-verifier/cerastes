// expect: rejected wrong_entity_for_attribute @ L9
// mode: c++20
// twin: tests/memory/not_invalidating_cpp.cpp
// Pins: 0058 item 5 (mirrors reject_owning_member_function.cpp): cerastes::not_invalidating on a member function is EDG's own refusal of the attribute (#1835, its registration has target p only), the first diagnostic, so the struct's member function (unsupported_declaration, M7) is not reached; the parameter position is reject_not_invalidating_member_function_parameter.cpp
// Pre-registered from record 0058 item 5: rejected wrong_entity_for_attribute at the member function (observed with build/cerastes verify)
#include <cerastes.h>
struct S {
  int x;
  CERASTES_NOT_INVALIDATING int *make();  // <- here
};
