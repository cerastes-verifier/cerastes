// Which post-parameter-list attribute does the IL keep? One entity per case,
// attribute names say where each was written: d = first declaration,
// r = redeclaration, f = definition. (Compile as C++20.)
int c1(int x) [[cerastes::c1d]];                          // case 1: decl, def
int c1(int x) [[cerastes::c1f]] { return x; }
int c2(int x) [[cerastes::c2d]];                          // case 2: decl, def without attribute
int c2(int x) { return x; }
int c3(int x) [[cerastes::c3d]];                          // case 3: decl, redecl, def without
int c3(int x) [[cerastes::c3r]];
int c3(int x) { return x; }
int c4(int x) [[cerastes::c4d]];                          // case 4: decl, redecl, def
int c4(int x) [[cerastes::c4r]];
int c4(int x) [[cerastes::c4f]] { return x; }
int c5(int x) { return x; }                               // case 5: def only
int c5b(int x) [[cerastes::c5f]] { return x; }
template <class T> T t1(T v) [[cerastes::t1d]];           // template case 1: decl, def
template <class T> T t1(T v) [[cerastes::t1f]] { return v; }
template <class T> T t2(T v);                             // template case 2: decl without, def
template <class T> T t2(T v) [[cerastes::t2f]] { return v; }
template <class T> T t3(T v) [[cerastes::t3d]];           // template case 3: decl, redecl, def
template <class T> T t3(T v) [[cerastes::t3r]];
template <class T> T t3(T v) [[cerastes::t3f]] { return v; }
struct M {
  int m1(int x) [[cerastes::m1d]];                        // member: in-class decl, out-of-class def
};
int M::m1(int x) [[cerastes::m1f]] { return x; }
int use() { return c1(1) + c2(1) + c3(1) + c4(1) + c5(1) + c5b(1) + t1(1) + t2(1) + t3(1) + M().m1(1); }
