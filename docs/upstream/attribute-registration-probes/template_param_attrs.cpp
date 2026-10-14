// Attributes on template parameters. Line 4 is a non-type parameter; lines 6,
// 8, 10 are type parameters. Compile with --c++20 (Apple clang rejects the
// type-parameter forms too).
template <[[cerastes::np]] int N> int f1() { return N; }
int use1() { return f1<1>(); }
template <[[cerastes::tp1]] class T> T f2(T v) { return v; }
int use2() { return f2(1); }
template <class [[cerastes::tp2]] T> T f3(T v) { return v; }
int use3() { return f3(1); }
template <[[cerastes::tp3]] typename T> T f4(T v) { return v; }
int use4() { return f4(1); }
