// expect: frontend-ok
// M3 probe P7: resolved calls, argument conversions, default arguments, a callee without a body.
int add(int a, int b) { return a + b; }
int nobody(int a);
signed char narrow(signed char c);
long wide(long l);
int with_default(int a, int b = 5, long c = 7L);
int with_default(int a, int b, long c) { return a + b + (int)c; }
void v(void);
int overloaded(int);
int overloaded(long);
int callers(int x, short s, unsigned char uc, long l) {
  int r = add(x, 1);
  r = add(s, uc);
  r = nobody(x);
  r = narrow(x);
  r = (int)wide(x);
  r = with_default(x);
  r = with_default(x, 2);
  r = overloaded(x) + overloaded(l);
  r = add(add(x, x), add(1, 2));
  v();
  return r;
}
int dflt_expr(int a, int b = 2 + 3, int c = add(1, 2));
int dflt_user(int x) { return dflt_expr(x) + dflt_expr(x, 1, 2); }
