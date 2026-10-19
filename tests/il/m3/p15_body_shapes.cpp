// expect: frontend-warning expr_has_no_effect @ L14
// expect: frontend-warning expr_has_no_effect @ L15
// expect: frontend-warning expr_has_no_effect @ L16
// expect: frontend-warning divide_by_zero @ L20
// expect: frontend-warning expr_has_no_effect @ L20
// expect: frontend-warning used_before_set @ L24
// mode: c++20
// M3 probe P15: shapes of discarded expressions, self-initialization and a return in a [[noreturn]] function.
int sink(int);
int discards(int a, int b) {
  (void)a;
  (void)(a + b);
  (void)sink(a);
  a / 1;
  a + b;
  a;
  return a;
}
int div_discard(int a) {
  a / 0;
  return a;
}
int self_init(int a) {
  int x = x;
  return x + a;
}
