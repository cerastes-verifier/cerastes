// expect: golden
// mode: c23
// Inventory probe for check 5b: an attributed null statement `[[unreg_ns::n(...)]];`
// in different positions. PASS means each attribute is reported on a
// `stmk_empty` statement (walker line `stmt stmk_empty @ L:C attr [[unreg_ns::nK(...)]]`);
// the golden records which placements survive and which do not.
//   n1  first statement of a function body
//   n2  first statement of a nested block
//   n3  after a declaration
//   n4  after an expression statement
//   n5  last statement of the body
//   n6a, n6b  two in a row (not first)
//   n7a, n7b  two in a row, first in the body
//   n8  first statement of a loop body
//   n9  first statement of an if body
int f1(int x) { [[unreg_ns::n1(x)]]; return x; }
int f2(int x) { { [[unreg_ns::n2(x)]]; } return x; }
int f3(int x) { int y = x; [[unreg_ns::n3(y)]]; return y; }
int f4(int x) { x++; [[unreg_ns::n4(x)]]; return x; }
int f5(int x) { x++; return x; [[unreg_ns::n5(x)]]; }
int f6(int x) { x++; [[unreg_ns::n6a(x)]]; [[unreg_ns::n6b(x)]]; return x; }
int f7(int x) { [[unreg_ns::n7a(x)]]; [[unreg_ns::n7b(x)]]; return x; }
int f8(int x) { while (x > 0) { [[unreg_ns::n8(x)]]; --x; } return x; }
int f9(int x) { if (x) { [[unreg_ns::n9(x)]]; } return x; }
