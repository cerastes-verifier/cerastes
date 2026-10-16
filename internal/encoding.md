# Encoding: C/C++ → VIR → Viper

Developer reference for the encoder (plan.md: "Design VIR and document it in `encoding.md`"; it grows each
milestone). The decisions behind it are 0010 (integer semantics), 0011 (driver), 0012 (scalar encoding). This
file says *how*; those records say *what* and *why*. Status: M3 (scalar code). Sections marked *(M3-n)* are
filled in at that checkpoint.

## 1. Pipeline and layering

```
EDG IL ──(src/backend/encode.cpp: IL → VIR, all C/C++ knowledge, allow-lists, rejections)──▶ VIR
VIR ──(src/vir/check.cpp: typing rules; fails closed)──▶ checked VIR
VIR ──(src/vir/lower.cpp: integer semantics → obligations, conversions, wraps)──▶ lowered VIR
lowered VIR ──(src/viper/emit.cpp + prelude.cpp + posmap.cpp: text only, no semantics)──▶ .vpr + .vpr.map
.vpr ──(Silicon, src/driver/silicon.cpp)──▶ errors ──(position map, src/driver/report.cpp)──▶ C/C++ lines
```

- `src/vir` and `src/viper` do not include EDG headers and do not link `edg_config`. Only `src/backend` sees the IL.
- All C/C++ meaning lives in `encode.cpp` (what the program says) and `lower.cpp` (what the operations mean,
  0010). The emitter prints; it decides nothing. The soundness auditor reviews those two files hardest.

## 2. VIR (`src/vir/vir.h`)

Front-end agnostic; every node has a `SrcLoc`.

- `SrcLoc { file, line, column }`: the C/C++ position by the rule of 0006 item 10.
- `IntType { name, bits, is_signed }`, filled from EDG's target configuration (0010 item 2).
- `Type = Bool | Int(IntType) | Void`.
- `Expr` (pure, typed): `Const(decimal)`, `Local(id)`, `Formal(k)`, `Result`, `Old(e)`, `Unary{neg, plus, not}`,
  `Binary{add, sub, mul, div, rem, shl, shr, lt, le, gt, ge, eq, ne, land, lor}`, `Cond(c, a, b)`,
  `Convert(to, e, explicit)`, `BoolToInt(e)`, `IntToBool(e)`, `CallResult(tmp)`.
- `Stmt`: `Decl(id, type, init)`, `Assign(id, e)`, `Call(dst?, fn, args, loc)`, `If(c, then, else)`,
  `While(cond, invariants[], body)`, `Return(e?, loc)`, `Check(e, loc)`, `Block(stmts)`.
- `Function { id, c_name, params[], ret, requires[], ensures[]{expr, loc}, body?, verify_body, end_loc }`.
- `Program { functions[] }`.

`check.cpp` typing rules (fail closed, `unsupported_expression` at the node):
- arithmetic operands and result have the same `IntType` (EDG's casts make promotions explicit, 0010 item 6);
- shift result type = promoted left operand type (the count is typed on its own);
- comparisons: operands of one type; result `Bool` in C++, `Int(int)` in C (via `BoolToInt`);
- logical `!`, `&&`, `||` and the conditions of `if`/`while`/`for`/`?:`: operands `Bool`. In C the encoder
  reads an integer operand there as `IntToBool(e)` (`e != 0`, 0010 item 6, C's truth values) and gives the
  result `Int(int)` via `BoolToInt`; in C++ EDG's `bool` conversion is already in the IL;
- `Convert` is the only other node that changes a type.

Compound assignment, `++`/`--` and shift-assignment are expanded by the encoder (0010 item 6) into
`x = Convert(T_x, x' op e')`, where `x'` and `e'` are converted to the computation type the encoder computes;
the typing rules then check the expansion like any other expression.

## 3. Lowering (`src/vir/lower.cpp`)

Each expression becomes `(value, obligations[])`; an obligation is `{category, guard, cond, loc, note?}`.
Guards follow 0012 item 5. Per operation (0010):

| Operation | Value | Obligation |
|---|---|---|
| signed `a op b` (`+ - *`) | `a op b` | `T_MIN <= a op b <= T_MAX` (`overflow`) |
| unsigned `a op b` | `c_wrap(a op b, 2^N)` | — |
| signed unary `-a` | `-a` | `a != T_MIN` (`overflow`) |
| unsigned unary `-a` | `c_wrap(-a, 2^N)` | — |
| unary `+a` | `a` | — |
| `a / b`, `a % b` | `c_div(a,b)`, `c_rem(a,b)` | `b != 0` (`division-by-zero`); signed: `!(a == T_MIN && b == -1)` (`overflow`) |
| `a << n` | signed `a * c_pow2(n)`; unsigned `c_wrap(a * c_pow2(n), 2^N)` | `0 <= n < w` (`shift`); signed: `a >= 0` and the product in range (`shift`) |
| `a >> n` | `c_shr(a, n)` | `0 <= n < w` (`shift`) |
| `Convert` to signed `T` | `e` | `T_MIN <= e <= T_MAX` (`conversion`), omitted when the source range is inside `T`'s |
| `Convert` to unsigned `T` | `c_wrap(e, 2^N)` (omitted when the source range is inside) | — |
| `IntToBool(e)` / `BoolToInt(e)` | `e != 0` / `e ? 1 : 0` | — |

## 4. Viper shape *(M3-4)*

```
method f_f__3(p_x__1: Int) returns (res: Int)    // names abbreviated below
  requires -2147483648 <= x && x <= 2147483647          // type range of x
  requires <user requires, in order>
  ensures  -2147483648 <= res && res <= 2147483647      // type range of res
  ensures  <user ensures, in order>
{
  var x__1: Int := x                                    // parameters are copied (0012 item 2)
  assert <guard ==> obligation>                         // one per line, each in the map
  res := <value>
  assert <ensures clause 1 at this return>              // postcondition @ the return
  goto end
  assert false                                          // missing-return @ the closing brace
  label end
}
method f__3__wf(x: Int, res: Int)                       // contract well-formedness (0012 item 10)
  requires <type ranges only>
{ assert <obligations of each clause, guarded by the earlier clauses> }
```

Names: every emitted identifier has a prefix that says what it is, so none can collide with another or with a
Viper keyword: functions `f_<name>__<n>`, formals `p_<name>__<k>`, locals `v_<name>__<k>` (unique `n`, `k`),
temporaries `t__<k>`, prelude functions `c_*`, labels `l_*`. A `$` in a C/C++ identifier (clang mode allows it)
is written `_D_`; that cannot make two names equal, because the generated suffix `__<k>` is unique per entity.

Silicon assumes an assertion's condition after reporting that it may fail, so later checks on the same path
cannot fail because of the same cause. Tests therefore put one failure on each path.

## 5. The position map (`<file>.vpr.map`) *(M3-4)*

One line per emitted check, tab-separated:

```
<vpr line> <TAB> <viper error id> <TAB> <category> <TAB> <file> <TAB> <line> <TAB> <column> <TAB> <message> [<TAB> <note file>:<note line>:<note text>]
```

- `<viper error id>` is the id Silicon's message maps to (the table of `scripts/run_tests.py`: `assert.failed`,
  `postcondition.violated`, `call.precondition`, `invariant.not.established`, `invariant.not.preserved`, …).
- The driver matches each Silicon error by its `.vpr` line and id; no match is exit 3 (0011 item 11).
- The driver's classifier of Silicon's messages and the harness's (`_VIPER_ERRORS`) are two copies of one
  table pinned to the Silicon version; a self-test runs both on the same recorded outputs and requires
  agreement.

## 6. Linting (`scripts/lint_viper.py`) *(M3-7)*

Fails on: `inhale`/`assume` without `// cerastes:inhale-ok <reason>` (invariant 4); a bodyless method without
`// cerastes:trusted-contract <reason>` (0012 item 1); any `domain`, `axiom` or bodyless `function`; an `assert`,
`requires`, `ensures`, `invariant` or method-call line with no position-map entry; an empty input set.

## 7. Matching IL to VIR *(M3-1, M3-4)*

The IL facts each rule relies on (node kinds, fields, conversions) are recorded with evidence in
`internal/il-inventory.md`, section "M3: scalar code" (probes P1–P12). The ones the encoder depends on most:
- `int s = 0;` is a `stmk_decl` followed by a `stmk_init`: encode the initializer once, at the `stmk_init`.
- `static_assert` in a body is a `stmk_decl` of a static-assertion entry (not encoded).
- `break`/`continue` are compiler-generated `stmk_goto` + `stmk_label`: every `stmk_goto` is rejected.
- A `for` init is a compiler-generated block; `if (int x = …)` has an `enk_condition` node (rejected).
- Reaching the closing brace of a non-`void` function is a compiler-generated `stmk_return` without an
  expression (`main`: with the constant 0).
- Calls: operand 1 is the resolved `enk_routine`; each argument is wrapped in a compiler-generated conversion
  to its parameter type; default arguments are copies whose top node has `generated_default_arg` and whose
  positions are in the callee's declaration.
- Parameters: contracts use `enk_param_ref` (`{k,0}` in `requires`, `{k,1}` in `ensures`, the result binder
  `{1,0}`); bodies and statement annotations use `enk_variable`, linked to parameter `k` by
  `variable.assoc_param_type`.

**Positions of obligations.** An operator's obligation is reported at the operator node's `position` (the
operator token); a call's at its `position` (the `(`); a conversion's at the conversion node, or at the
enclosing initializer, assignment, argument or `return` when the conversion is compiler-generated; an
obligation that comes from a default argument at the call, with a note at the default argument. Inside a
`CERASTES_*` argument the original position (`orig_seq`/`orig_column`) is used (0006 item 10).
