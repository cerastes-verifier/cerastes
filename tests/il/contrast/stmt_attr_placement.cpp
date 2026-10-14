// expect: golden
// Contrast probe: a [[unreg_ns::...]] attribute on a statement must be
// reported on that statement's own stmk_* kind at its own position, and
// nothing may land on a neighbouring statement.  Each attribute name says
// where it is written: s_if on an `if`, s_do on a `do`/`while`, s_switch on a
// `switch`, s_block on a compound statement `{}`, s_expr on an expression
// statement.  Statements without an attribute (the `return`, the other
// expression statements, the case labels) must have no `stmt` line.  The
// goldens of check 5 attribute loops and null statements; this probe covers
// the other kinds.
// Positions (checked against --il_display): EDG gives the `if`, `do`,
// `switch` and expression statements the position of their leading
// attribute (column 3), but gives the compound statement the position of its
// `{` (column 32, after the attribute).  The golden records that as it is.

int place(int x) {
  int y = 0;
  [[unreg_ns::s_if(x > 0)]] if (x > 0) { y = 1; }
  y = y + 1;
  [[unreg_ns::s_do(y < 10)]] do { y = y + 2; } while (y < 10);
  y = y + 3;
  [[unreg_ns::s_switch(x)]] switch (x) {
    case 1: y = 4; break;
    default: y = 5; break;
  }
  y = y + 6;
  [[unreg_ns::s_block(y > 0)]] { y = y + 7; }
  y = y + 8;
  [[unreg_ns::s_expr(y > 0)]] y = y + 9;
  y = y + 10;
  return y;
}
