// expect: golden
// Inventory probe for rows C9, C10, D5, D9, D11 (operations in prototype
// bodies). PASS: each body shows the operator kind named in its comment with
// the operand types, so the checker can whitelist it.
#include <concepts>
// C10 eok_not, C9 eok_land / eok_lor
template <std::integral T> bool logic(T a, T b) { return !(a < b) && (a == b || a > 3); }
// D11 eok_question over T
template <std::integral T> T cond(T a, T b) { return a < b ? a : b; }
// D5 default argument that depends on T
template <std::integral T> T dflt(T a, T b = (T)1) { return (T)(a + b); }
// D9 address-of, dereference and a T& parameter
template <typename T> T deref(T *p, T &r) { T *q = &r; return *p + *q; }
int use() { int i = 1, j = 2; return logic(1, 2) + cond(1, 2) + dflt(1) + deref(&i, j); }
