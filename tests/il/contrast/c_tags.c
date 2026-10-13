// expect: golden
// mode: c23
// Contrast probe: in C, a tag type is written with its keyword.  The routines
// take and return `enum color`, `struct pt` and `union un`; the golden shows
// the types exactly as EDG renders them (with `enum`, `struct` and `union`),
// not as bare names.  The names differ from the C++ probes so that a stale
// shared rendering cannot hide.

enum color { RED, GREEN };
struct pt { int x; int y; };
union un { int i; float f; };

enum color next_color(enum color c) { return c == RED ? GREEN : RED; }

struct pt swap_pt(struct pt p) {
  struct pt q = {p.y, p.x};
  return q;
}

union un pass_un(union un v) { return v; }

int tag_sum(enum color c, struct pt p, union un v) { return (int)c + p.x + v.i; }
