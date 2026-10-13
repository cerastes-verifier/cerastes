// expect: golden
// Contrast probe: types without a name.  Routines take and return an unnamed
// enum and an unnamed struct (reached through decltype of a variable, and
// through a typedef that names an unnamed struct only for linkage).  The
// walker must print a rendering for them, never a `<tk_...>` placeholder for
// the type kind; what EDG's default rendering is for an unnamed class or enum
// is recorded in the golden, not assumed here.

struct { int a; } anon_s;
enum { AE1, AE2 } anon_e;
typedef struct { int q; } TS;

int take_s(decltype(anon_s) s) { return s.a; }
int take_e(decltype(anon_e) e) { return e; }
decltype(anon_e) give_e() { return AE2; }
decltype(anon_s) give_s() { return anon_s; }
int take_ts(TS t, TS* tp) { return t.q + tp->q; }
