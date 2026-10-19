// expect: frontend-warning implicit_return_from_non_void_function @ L19
// mode: c++20
// M3 probe P11: statement shapes.
int f(int n);
int loops(int n) {
  int s = 0;
  for (int i = 0; i < n; i++) s += i;
  for (;;) { break; }
  for (int j = 0, k = 1; j < n; ++j, ++k) { s += k; }
  while (n > 0) { n--; if (n == 3) continue; }
  if (int x = f(n)) s += x; else s -= 1;
  if (n) { s = 1; } else if (s) s = 2; else { s = 3; }
  { int inner = s; s = inner + 1; }
  int uninit;
  uninit = 4;
  return s + uninit;
}
int main() { f(1); }
int falls_off(int a) { if (a) return 1; }
void ret_void(int a) { if (a) return; a = 1; }
signed char ret_conv(int a) { return a; }
int sa(int n) { static_assert(sizeof(int) == 4, "int"); return n; }
