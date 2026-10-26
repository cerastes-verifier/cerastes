// expect: golden
// Attributes on the function types of typedefs, aliases, variables and parameters.  They are reached through their own
// entities (a typedef, a variable, a parameter), not through a routine's type, and the walker lists each as `on type
// <unnamed>`; the redeclarations' function types that the source sequence lists also reach are not listed again
// (decision 0024 item 1).  The golden was checked against the listing of the build without source sequence lists
// (t1 to t4 on a type, t5 on its routine, same positions) before it was generated.
typedef int fn_t(int x) [[unreg_ns::t1(x > 0)]];
using fp_t = int (*)(int y) [[unreg_ns::t2(y > 0)]];
int (*gp)(int z) [[unreg_ns::t3(z > 0)]] = nullptr;
int call(int (*p)(int w) [[unreg_ns::t4(w > 0)]], int v) { return p(v); }
int h(int q) [[unreg_ns::t5(q > 0)]] { return q; }
