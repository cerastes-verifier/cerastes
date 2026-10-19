// expect: frontend-ok
// mode: c23
// M3 probe P5 (C23): bool, _Bool, true/false, comparison and logical results, conversions to bool.
int cmp(int a, int b) { return a < b; }
int logic(int a, int b) { return a && b; }
int lnot(int a) { return !a; }
bool as_bool(int a) { return a; }
bool as_bool2(int a, int b) { bool r = a < b; return r; }
_Bool old_bool(_Bool b) { return !b; }
int from_bool(bool b) { return b + 1; }
bool consts(void) { return true; }
bool ff(void) { return false; }
int cond_c(int a, int b) { if (a) return 1; if (a < b) return 2; while (a && b) a = a - 1; return 0; }
bool inc_bool(bool b) { ++b; b++; return b; }
