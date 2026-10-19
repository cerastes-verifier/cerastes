// expect: golden
// mode: c++20
// M3 probe P13 (C++20): recognized attributes in verified code and where each lands in the IL.
[[noreturn]] void nr_decl(int);
[[noreturn]] void nr_def(int a) { for (;;) { (void)a; } }
[[nodiscard]] int nd(int a) { return a; }
[[deprecated("old")]] int dep(int a) { return a; }
[[gnu::always_inline]] inline int ai(int a) { return a; }
int ai2(int a) __attribute__((always_inline));
int ai2(int a) { return a; }
__attribute__((const)) int cn(int a) { return a; }
__attribute__((pure)) int pu(int a) { return a; }
void cleanup_fn(int *);
int with_params([[maybe_unused]] int a, int b [[maybe_unused]]) { return b; }
struct [[nodiscard]] Tag { int v; };
struct [[deprecated]] Old { int v; };
enum [[deprecated]] En { EnA };
int stmts(int x) {
  [[maybe_unused]] int unused = 0;
  int cl __attribute__((cleanup(cleanup_fn))) = 1;
  [[assume(x > 0)]];
  __attribute__((assume(x > 1)));
  if (x > 3) [[likely]] { x = 1; } else [[unlikely]] { x = 2; }
  switch (x) { case 1: x = 2; [[fallthrough]]; case 2: x = 3; break; default: break; }
  [[]];
  return x + cl;
}
