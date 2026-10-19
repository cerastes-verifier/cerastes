// expect: frontend-ok
// M3 probe P8: the positions an operator node carries (operator token) against its start and end.
int g(int a, int b)
{
  int r = (a + b) * 2;
  r = -r;
  r = g(a, b);
  return r > 0 ? r : -r;
}
