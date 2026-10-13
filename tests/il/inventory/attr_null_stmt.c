// expect: golden
// mode: c23
// Inventory probe for check 5b: an attributed null statement `[[cerastes::n(...)]];`
// in different positions. PASS means each attribute is reported on a
// `stmk_empty` statement (walker line `stmt stmk_empty @ L:C attr [[cerastes::nK(...)]]`);
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
int f1(int x) { [[cerastes::n1(x)]]; return x; }
int f2(int x) { { [[cerastes::n2(x)]]; } return x; }
int f3(int x) { int y = x; [[cerastes::n3(y)]]; return y; }
int f4(int x) { x++; [[cerastes::n4(x)]]; return x; }
int f5(int x) { x++; return x; [[cerastes::n5(x)]]; }
int f6(int x) { x++; [[cerastes::n6a(x)]]; [[cerastes::n6b(x)]]; return x; }
int f7(int x) { [[cerastes::n7a(x)]]; [[cerastes::n7b(x)]]; return x; }
int f8(int x) { while (x > 0) { [[cerastes::n8(x)]]; --x; } return x; }
int f9(int x) { if (x) { [[cerastes::n9(x)]]; } return x; }
