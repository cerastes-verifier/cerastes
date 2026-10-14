// expect: golden
// Inventory probe for check 5: attribute before a class declaration.
// PASS: the golden records which of these are in the IL.
//   X1  prefix of a class definition that declares no entity (appertains to
//       nothing: expected to be dropped, with no diagnostic from this probe)
//   X2  prefix of a class definition that also declares a variable
//   X3  after the class-key (appertains to the class)
//   X4  prefix of an enum definition that declares no entity
[[unreg_ns::X1(1)]] struct S1 { int a; };
[[unreg_ns::X2(2)]] struct S2 { int a; } s2var;
struct [[unreg_ns::X3(3)]] S3 { int a; };
[[unreg_ns::X4(4)]] enum E4 { e4 };
int use() { return s2var.a; }
