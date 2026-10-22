# EDG IL inventory (M1)

**Status: complete. Gate G1: PASS (see "Gate G1 verdict"). Walker output is cross-checked against `il_display`, and the goldens are in `tests/il/`.**
All excerpts below come from `build/cerastes-fe` with
`DEFAULT_INSTANTIATION_MODE tim_used`, `AUTOMATIC_TEMPLATE_INSTANTIATION 0`,
`PARENS_IN_IL 0`, `PROTOTYPE_INSTANTIATIONS_IN_IL 1`,
`ALL_TEMPLATE_INFO_IN_IL 1`, `RECORD_UNRECOGNIZED_ATTRIBUTES 1`
(`--dump_configuration`).

## Purpose

Gate G1 asks whether EDG's prototype-instantiation IL carries what the generic
proof of M5 needs. VeriFast answers that question with Clang AST queries
(`GenericFunctionChecker` and friends in `src/cxx_frontend/ast_exporter/`). This
file maps every such query to the EDG IL field that answers it, with
`--il_display` evidence, and records the five G1 checks.

## Setup

- EDG pin: `third_party/edg` at `9847356d` (upstream `edgcpp/compiler`) when the evidence was produced. The pin moved to `034385ba` on 2026-10-14 (M2); nothing under `src/` changed and every golden was byte-identical (0001, "Pin history").
- Configuration: the cerastes configuration of decision
  [0002](decisions/0002-backend-integration.md): `DO_IL_LOWERING=0`,
  `PROTOTYPE_INSTANTIATIONS_IN_IL=1`, `ALL_TEMPLATE_INFO_IN_IL=1`,
  `RECORD_UNRECOGNIZED_ATTRIBUTES=1`, `EXTRA_SOURCE_POSITIONS_IN_IL=1`.
- Evidence command (run from the repository root, only with `build/cerastes-fe`;
  stock `cpfe` lowers the IL and `cpfe-cp` lacks `ALL_TEMPLATE_INFO_IN_IL`, so
  neither is evidence):

  ```
  scripts/cerastes-fe.sh --c++20 --il_display <probe>
  ```

- Attribute namespace in the probes (M2): the probes whose made-up attribute
  names exercise EDG's unrecognized-attribute path
  (`tests/il/g1/check5_attributes.cpp`, `tests/il/inventory/attr_*.cpp|.c`,
  `tests/il/contrast/{alias_attribute,lambda_bodies,ordinary_attributes,stmt_attr_placement,variable_attributes}.cpp`,
  and their goldens) used `[[cerastes::...]]` when this inventory was written.
  M2 registers `cerastes` as a closed attribute namespace, which would turn
  those made-up names into errors, so since M2 the probes use `[[unreg_ns::...]]`
  (a namespace nobody registers). The change is a same-length substitution
  `cerastes` to `unreg_ns`; every line of every source and golden is otherwise
  byte-identical. The excerpts in this file keep the original `cerastes::`
  spelling; read `cerastes::x` there as `unreg_ns::x` in the current probes and
  goldens. (`tests/il/contrast/line_directive.cpp` is not renamed.)
- VeriFast source: `~/Documents/verifast`, branch `generic2`, commit
  `0fb63506a51cae559ae92348eb54ba7f7be0bf6d`. File names below are relative to
  `src/cxx_frontend/ast_exporter/`.
- Probes: `tests/il/g1/check{1..5}_*.cpp` (one per check) and
  `tests/il/verifast-plain/*.cpp` (the six VeriFast template tests with the
  `//@` annotations and `//~` markers blanked; line N of a copy is line N-6 of
  the original in `tests/differential/verifast/`).
- `position.seq` in `il_display` is a global source-sequence number, not a line
  number. In a file with no `#include` it equals the line (`check5`,
  `attr_*`, `specializations`); with includes it is offset (`15120` is the first
  line after `<concepts>` in the `check*` probes). Cite probe lines from the
  source, `seq` for matching dump entries.
- Extra probes (`tests/il/inventory/`): `attr_postfunc.cpp`,
  `attr_classtmpl.cpp`, `ops.cpp`, `specializations.cpp`.
- `il_display` prints an entity only when it is reachable from the lists it
  walks. Attribute entries referenced from statements (`a_statement.attributes`)
  are NOT printed (the pointer is shown, the entry is not), so token content of
  statement attributes needs the walker, not `il_display`.
- **Excerpts are condensed.** Field names and values are verbatim from
  `il_display`; the layout is condensed (fields from several lines of a block
  are joined onto one line, indentation shows nesting of expression nodes, and
  `...` stands for an address, e.g. `file-scope type@7b0e...`). Nothing is
  paraphrased inside a `field: value` pair. Walker lines are verbatim from
  `--cerastes_walk`.
- **Notation `type[kind]`.** In the query table a bracketed `[kind]` after a
  type name (for example `T#(1,1)[tptk_param]`, `[tk_pointer]`, `[tptk_unknown]`)
  is added by this document: it is the `kind:` line (for a template-parameter
  type also its `kind: tptk_...` line) that `il_display` prints on the type
  entry. The type name itself is as `il_display` prints it, and `<unnamed@...>`
  is a `tk_template_param` type with `kind: tptk_unknown`.
- `il_def.h:NNN` means `third_party/edg/src/il_def.h`, line NNN, at the pin.

## Gate G1 checks

Verdicts: PASS / FAIL / UNCLEAR. Dumps were saved outside the repo
(scratchpad `il/*.txt`); reproduce with the command above. Addresses differ
between runs.

### Check 1: type_kind of `v + 1`, and is the `(T)` cast its own node? **PASS**

- Probe: `tests/il/g1/check1_inc.cpp` (`inc`, `plus_local`, `const_cast_c`,
  `static_cast_v`, `less10`, `sum2`). Regression test (walker golden):
  `tests/il/g1/check1_inc.cpp.golden`.
- IL fields: `an_expr_node.variant.operation.{kind,type_kind}`
  (`il_def.h:15121-15135`), node `type` (`il_def.h:14907`),
  `a_type.variant.template_param.kind` (`il_def.h:10896`, `9289`).
- Evidence, `il_display` of the prototype `inc` (`return (T)(v + 1);`), the
  statement `stmk_return` and below:

  ```
  kind: enk_operation   operation.kind: eok_cast   operation.type_kind: tk_template_param   type: ...: T#(1,1)
    kind: enk_operation   operation.kind: eok_add   operation.type_kind: tk_template_param   type: ...: <unnamed@...>   is_parenthesized: TRUE
      kind: enk_variable   variable: ...: v   type: ...: const T#(1,1)
      kind: enk_constant   constant: ...: 1   type: ...: int      (constant block: kind: ck_integer   integer_value: 1)
  ```

  The `<unnamed@...>` type is a `tk_template_param` whose `kind: tptk_unknown`.
  The instance `inc<int>` has `operation.type_kind: tk_integer` and
  `type: ...: int` on both operations.
- Walker line for the same node (`--cerastes_walk`, golden
  `check1_inc.cpp.golden`):

  ```
  op eok_cast type_kind=tk_template_param type=T @ L9:27
  op eok_add type_kind=tk_template_param type=<tptk_unknown> @ L9:33
  ```

  and for `(T)100` in `const_cast_c` (`il_display`: `kind: enk_constant` node at
  position `15126:27`, constant `kind: ck_template_param`, `kind: tpck_expression`,
  whose `expr:` is `kind: enk_operation`, `operation.kind: eok_cast`,
  `operation.type_kind: tk_unknown`, position `15126:27`, over
  `constant: ...: 100`):

  ```
  tconst tpck_expression type=T @ L15:27
  op eok_cast type_kind=tk_unknown type=T @ L15:27
  ```

  This matters for M5's fail-closed rule: a cast to `T` of a literal is an
  `operation.type_kind: tk_unknown` operation, so a rule that rejects
  `tk_unknown` operations must treat a `tpck_expression` constant whose `expr`
  is such a cast as accepted (or accept `eok_cast` to `T` regardless of
  `type_kind`). The same `eok_cast type_kind=tk_unknown` shows up for the
  default argument `(T)1` (`ops.cpp` `dflt`, `@ L11:46`) and the `T acc = 0` /
  `T i = 0` initializers (`check2`, `check5`: `@ L11:11`, `@ L46:9`).

- Answer: `v + 1` has `type_kind == tk_template_param` (not `tk_unknown`); the
  `(T)` cast is a separate `eok_cast` node (also for `static_cast<T>`:
  `static_cast_v`).
- Gotchas found:
  - the node `type` of a dependent operation is the `<unnamed>`
    `tk_template_param` with `kind: tptk_unknown`, not `T` and not `bool` (a
    dependent `v < 10` is `tptk_unknown`, `less10`);
  - `(T)100` (`const_cast_c`) is NOT an `eok_cast` at top level. It is
    `kind: enk_constant` with a constant of `kind: ck_template_param`,
    `kind: tpck_expression`, `type: ...: T#(1,1)`; the cast is in the
    constant's `expr:` with `operation.type_kind: tk_unknown`. The walker prints
    the `tconst` and the nested `op eok_cast ... tk_unknown` (walker lines
    above);
  - `auto w = v + 1;` (`plus_local`) leaves `w` with `type: ...: auto`; the
    initializer is in a `stmk_init` / `dynamic-init`, which the walker also
    walks (`op eok_add ... @ L12:38`);
  - parentheses are the flag `is_parenthesized: TRUE`, not an `eok_parens`
    node.

### Check 2: never-instantiated template has a prototype routine with a full body **PASS**

- Probes: `tests/il/g1/check2_uninstantiated.cpp` (`never_used` vs `used`),
  `tests/il/verifast-plain/integral.cpp` (`uninstantiated`, line 102 of the
  copy). Goldens: `tests/il/g1/check2_uninstantiated.cpp.golden`,
  `tests/il/verifast-plain/integral.cpp.golden`.
- IL fields: `a_template.prototype_instantiation.routine` (`il_def.h:17391`),
  routine `is_prototype_instantiation` (`il_def.h:12546`),
  `function_def_number` (`il_def.h:12245`), scope `assoc_block`
  (`il_def.h:18444`).
- Evidence: `never_used` has exactly one routine, a prototype. Its block:
  `type: ...: T#(1,1) (T#(1,1))`, `function_def_number: 32`,
  `memory_region: 33`, `defined: TRUE`, `is_prototype_instantiation: TRUE`,
  `is_template_function: TRUE`, `assoc_template: ...: template@...`,
  `template_arg_list:` entry `type: ...: T#(1,1)`. Its function region
  (`Intermediate language for memory region 33 (function "never_used")`) has a
  scope with `kind: sck_function`, `assoc_block: ...: statement@...`, whose
  statements are `stmk_block`, `stmk_decl`, `stmk_init`, `stmk_for` (with
  `stmk_if` in its body), each with its expression nodes
  (`operation.kind: eok_bool_cast`, `eok_lt`, `eok_assign`, `eok_cast`,
  `eok_add`, `eok_subtract`, and `eok_pre_incr` for `++i`).
- Walker: `routine never_used prototype body @ L10:3` followed by 13 `op` and
  3 `tconst` lines (`eok_cast` 4, `eok_assign` 2, `eok_bool_cast` 2, `eok_lt` 2,
  `eok_add`, `eok_subtract`, `eok_pre_incr`).
- `integral.cpp` `uninstantiated` is the same: one prototype with body, no
  instance (`op eok_cast` over nested `eok_add` of `v`). The instantiated
  contrast `used` has the prototype and an instance whose type is
  `type: ...: int (int)` with `template_arg_list: type: ...: int`
  (`routine used instance body @ L20:3`).

### Check 3: instance routines carry `assoc_template` and `template_arg_list` **PASS**

- Probe: `tests/il/g1/check3_instances.cpp`; golden
  `tests/il/g1/check3_instances.cpp.golden`.
- IL fields: `a_routine.assoc_template` (`il_def.h:13176`),
  `a_routine.template_arg_list` (`il_def.h:13166`), `a_template_arg.kind`,
  `variant.type`, `explicitly_specified` (`il_def.h:7683-7700`).
- Evidence (instances; every one has `assoc_template: ...: template@...` equal
  to the template of its prototype):

  | call | instance routine `type:` | `template_arg_list:` entries |
  |---|---|---|
  | `inc(5L)` deduced | `long (long)` | `type: ...: long` |
  | `inc<unsigned char>(3)` | `unsigned char (unsigned char)` | `type: ...: unsigned char` |
  | `pick<int>('x')` | `char (char)` | `type: ...: int`, `explicitly_specified: TRUE`; then `type: ...: char` |
  | `twice_inc((short)1)` | `short (short)` | `type: ...: short` |
  | created from inside `twice_inc<short>` | `short (short)` (`inc`) | `type: ...: short` |

  `pick` also prints `expl_template_arg_list_used: TRUE`. The prototype
  routines (`inc`, `pick`, `twice_inc`) have `is_prototype_instantiation: TRUE`
  and ALSO an `assoc_template` and a `template_arg_list` listing the
  template's own parameters (`type: ...: T#(1,1)`).
- Walker: `routine pick instance body @ L13:3` with
  `template pick args <T=int, U=char>`; `routine inc instance ... args <T=long>`,
  `<T=unsigned char>`, `<T=short>`.
- Gotchas: an instance is recognized by the ABSENCE of
  `is_prototype_instantiation`. The prototype body of `twice_inc` contains only
  unresolved `eok_call`s (`op eok_call type_kind=tk_unknown ...`, callee
  `tconst tpck_unknown_function`); no `inc<T>` instance comes from it.
  Instances exist only for what is used, because of `tim_used`.

### Check 4: `std::integral` reachable from all four constraint locations **PASS**

- Probe: `tests/il/g1/check4_constraints.cpp`; golden
  `tests/il/g1/check4_constraints.cpp.golden`.
- IL fields: `a_template_param_type_supplement.constraint.type_constraint`
  (`il_def.h:9386`), `a_template_decl.constraint.requires_clause`
  (`il_def.h:17234`), `a_routine.trailing_requires_clause`
  (`il_def.h:13107`), `a_template_parameter.is_abbreviated`
  (`il_def.h:17168`), `an_expr_node.variant.concept_id.{concept_template,args}`
  (`il_def.h:15710-15717`), `is_type_constraint` (`il_def.h:15095`).
- Evidence: all four resolve to `kind: enk_concept_id` whose
  `concept_id.concept_template` is a template with `kind: templk_concept` and
  `name: ...: "integral"`, in a `sck_namespace` scope whose `assoc_namespace`
  is `__1` (inside `std`):

  | form | where | `concept_id.args` |
  |---|---|---|
  | `f1` `template<std::integral T>` | `constraint.type_constraint` of `T`'s type entry; `is_type_constraint: TRUE` | `NULL` |
  | `f2` `requires std::integral<T>` | `template-decl` requires clause: `requires-clause` `constraint:` | `type: ...: T#(1,1)`, `explicitly_specified: TRUE` |
  | `f3` trailing `requires` | routine `trailing_requires_clause: ...: requires-clause@...` | `type: ...: T#(1,1)`, `explicitly_specified: TRUE` |
  | `f4` `std::integral auto x` | `template-parameter` with `is_abbreviated: TRUE`, no `name`, `ptr:` type `std::__1::integral auto`; its type has `constraint.type_constraint` | `NULL` |
  | `f5` `A<T> && A<U>` | requires clause: `operation.kind: eok_land` (`operation.type_kind: tk_integer`, `do_not_interpret: TRUE`) over two `enk_concept_id` | `T`, `U`, explicit |

- Walker: `constraint type-constraint T std::__1::integral` (f1),
  `constraint requires std::__1::integral<T>` (f2),
  `constraint trailing std::__1::integral<T>` (f3),
  `constraint abbreviated #0 std::__1::integral` (f4),
  `constraint requires std::__1::integral<T> && std::__1::integral<U>` (f5).
- Gotchas: the concept lives in the libc++ inline namespace (`std::__1`); a
  type-constraint's first argument is not in `args`; the abbreviated
  parameter has no name (the walker labels it `#0`: a zero-based position
  label, while `coordinates.position` is 1-based).

### Check 5: unrecognized `[[cerastes::...]]` attributes survive

Probes: `tests/il/g1/check5_attributes.cpp`,
`tests/il/inventory/attr_postfunc.cpp`, `tests/il/inventory/attr_classtmpl.cpp`,
`tests/il/inventory/attr_tokens.cpp`, `tests/il/inventory/attr_prefix_redecl.cpp`.
Goldens: the `.golden` files next to them .

IL fields: `an_attribute` (`il_def.h:3157`): `kind: unrecognized`, `family:
std`, `syntactic_location`, `name`, `namespace_name`, `arguments` (`aak_raw_token`
list ended by `aak_empty`, `il_def.h:2750-2830`), `position`/`end_position`;
owners `a_source_correspondence.attributes` (`il_def.h:3610`) and
`a_statement.attributes` (`il_def.h:16541`).

Every attribute is `kind: unrecognized`, `family: std`,
`namespace_name: ...: "cerastes"`, `is_invalid_namespace: TRUE` (the #2803
case), with `position` and `end_position`. Raw tokens print as separate
`aak_raw_token` entries (`x`, `>`, `0`, `,`, ...). EDG re-spells the tokens
(`attr_tokens.cpp`, golden `attr_tokens.cpp.golden`): the source
`[[cerastes::t1(0x1F, 9u, 1.50, 1E3, 'a', "s", 07, 10UL)]]` prints (both in
`il_display` `token:` entries and in the walker) as
`0x1f , 9U , (1.5) , (1000.0) , 'a' , "s" , 0x7 , 10UL`: hex digits lower-cased,
the integer suffix upper-cased, floating literals normalized and parenthesized,
octal turned into hex. The token text is not the source text.

**Sub-result 5a: prefix attribute on a function declaration: PASS**
- Plain function (`a1`, `b0`): attached to the routine
  (`source_corresp.attributes`), `syntactic_location: prefix`, tokens
  `x > 0 , y + 1`; walker `attr [[cerastes::a1(x > 0 , y + 1)]] on routine @ L24:3`.
- Function template (`b1`, line 39): attached to the prototype routine AND to
  the instance routine `tmpl<int>`; walker
  `attr [[cerastes::b1(x > 0 , x + 1)]] on routine @ L39:3` under both
  `routine tmpl prototype` and `routine tmpl instance`.
- Class-template member function (`m1`, `attr_classtmpl.cpp`): on the
  prototype `Box<T>::get1` and on `Box<int>::get1`; same for the data member
  `m2` (field `v`) and the class (`m3`, `syntactic_location: tag name`)
  in `il_display`; the walker prints `m2`/`m3` as top-level
  `attr ... on field Box<T>::v` / `on field Box<int>::v` / `on type Box<T>` lines and they are in the golden. The drop predicted from
  `attribute.c:10906` was not observed.

**Sub-result 5a-redecl: prefix attributes merge across redeclarations: PASS**
- Probe `tests/il/inventory/attr_prefix_redecl.cpp` (golden `attr_prefix_redecl.cpp.golden`). In
  `il_display` the attribute entries of the declarations are chained through
  `next:` and hang off the entity's `attributes:`:
  - plain function `p1`, declared with `r1` (line 10) and `r2` (line 11) and
    then defined: routine `p1` has `attributes: ...: attribute@A`, `A` is `r1`
    with `next:` pointing to `r2`;
  - function template `p2` (`r3` line 13, `r4` line 14, then defined,
    instantiated by `p2(2)`): the prototype routine and the instance routine
    each have `r3` -> `r4` (two separate chains, same positions 13:25 and 14:25);
  - member `S::m`, declared in the class with `r5` (16:14) and defined outside
    with `r6` (17:3): the routine has `r5` -> `r6`.
- Walker (current build): `routine p1 ... attr r1 on routine @ L10:3`, `attr r2
  on routine @ L11:3`; the same pair for `p2` prototype and instance, and
  `r5`, `r6` for `S::m`. Golden: `attr_prefix_redecl.cpp.golden`.
- Meaning: prefix attributes on a function are safe across redeclarations,
  templates and out-of-class member definitions; they accumulate in
  declaration order. This contrasts with 5c (after the parameter list).

**Sub-result 5b: attributes on loop and null statements: PASS**
- Statement entries carry `attributes: ...: attribute@...` on exactly the
  statements that have one in the source (`il_display` prints the pointer but
  not the entry): `stmk_while` at 30:3, `stmk_for` 31:3, `stmk_empty` 32:3 in
  `g`; 47:3, 48:3, 49:3 in the prototype `tmpl` and again in the instance
  `tmpl<int>`; 58:5 and 59:5 in `Box<T>::get` and `Box<int>::get`. These are
  13 statements; the walker prints 13 `stmt ... attr` lines with the same
  kinds and positions.
- Token content, by differential method (the entries are not printed by
  `il_display`): a copy of `check5_attributes.cpp` was changed so `a3` became
  `[[cerastes::zz3(q == 7)]]` and `b4` became `[[cerastes::yy4(k, 2, 3)]]`.
  The walker output changed in exactly these lines and no others:
  `stmt stmk_while @ L30:3 attr [[cerastes::zz3(q == 7)]]` and, for `b4` in
  both the prototype and the instance, `stmt stmk_for @ L48:3 attr
  [[cerastes::yy4(k , 2 , 3)]]`. The only other changed lines were three `op`
  / `tconst` positions on line 48 (`L48:39`, `L48:42`, `L48:44` became `L48:41`,
  `L48:44`, `L48:46`), because the new attribute text is two characters
  longer than the old one on that line. In the `il_display` dumps of the
  original and the modified file, the 13 statements with non-NULL
  `attributes` have identical kinds and positions. So the walker's names and
  tokens come from the attribute entries themselves, and their placement
  matches `il_display`.
- Not covered by this method: `a4`, `a5`, `b3`, `b5`, `c2`, `c3` token content
  (same code path, not separately varied). The null-statement tokens in
  `attr_null_stmt.cpp` are printed by the walker and were not varied.

**Sub-result 5b-null: an attributed null statement `[[cerastes::n(...)]];` survives in every placement tried: PASS**
- Probes `tests/il/inventory/attr_null_stmt.cpp` and `attr_null_stmt.c`
  (`// mode: c23`), goldens next to them. In `il_display` every one of the
  attributed `stmk_empty` statements has `attributes: ...: attribute@...`
  (non-NULL), at the position the walker prints:
  - first statement of a function body (`n1`, `f1`: `stmk_empty` 17:17),
  - first statement of a nested block (`n2`, 18:19),
  - after a declaration (`n3`, 19:28), after an expression statement (`n4`,
    20:22), last statement, after `return` (`n5`, 21:32),
  - two in a row, not first (`n6a`, `n6b`, 22:22 and 22:44) and first in the body
    (`n7a`, `n7b`, 23:17 and 23:39),
  - first statement of a `while` body (`n8`, 24:33) and of an `if` body (`n9`,
    25:26),
  - inside a function template: prototype and instance both (`n10` first,
    `n11` after a statement, 26:35 and 27:42, twice each);
  - in C mode (c23): all 11 non-template placements survive; the statement
    position in C is the `;` (for example `n1` at 16:36), in C++ it is the
    `[[` (17:17).
- Extra shapes (scratch probes, not in the repository): the attributed null
  statement also survives after a label, after `case 1:`, as the dependent
  statement of `if`, `else` and `for(;;)`, with no arguments (`[[cerastes::Q]]`,
  `[[cerastes::Q()]]`), in a constructor, a member function, a lambda,
  a function-try-block and a `constexpr` function, in C++14/17/20/23 and C11/17/23.
- Reported finding NOT reproduced: edg-backend reported that
  `int b(int x){[[cerastes::Q(1)]];return x;}` has no attribute in the IL. With
  the current build `--cerastes_walk` prints
  `stmt stmk_empty @ L1:14 attr [[cerastes::Q(1)]]`, and `il_display` shows the
  `stmk_empty` statement (`position.seq: 1`, `position.column: 14`) with
  `attributes: file-scope attribute@...`. Likely cause of the report: the
  attribute ENTRY itself is not printed by `il_display` (see 5b), so a search of
  `il_display` for `Q` finds nothing, while the statement's `attributes:` pointer
  is set.
- Why it survives (parser, `statements.c`): at statement start the parser scans
  leading `[[ ... ]]` attributes into `struct_stmt_stack_top().prefix_attributes`
  (`:7819-7822`, `:8415-8421`) before deciding whether a declaration follows.
  `[[...]];` is not a declaration start, so it takes the statement path and the
  attributes stay attached to the statement; they are handed to a declaration
  (`unscan_attributes`, `:8075-8118`, `:8437-8442`) only on the declaration
  paths. A leading attribute therefore cannot be mistaken for an
  attribute-declaration, and the first statement of a block is not special.
- M2 consequence: `check(p)` on a null statement is safe in every position
  tried, including the first statement of a body, in templates (prototype and
  instance) and in C. The caveat is token content: `il_display` cannot show it
  (the walker can), and 5b's differential method applies.

**Sub-result 5b-class: prefix attribute on a class or enum definition: expected drop confirmed**
- `tests/il/inventory/attr_class_prefix.cpp`: `[[cerastes::X1(1)]] struct S1 {...};`
  (no declared entity) and `[[cerastes::X4(4)]] enum E4 {...};` are NOT in the IL:
  `il_display` has no attribute entry for them, and the walker prints no `attr`
  line for them (warning #2803 is still printed, so the parser saw them).
  `[[cerastes::X2(2)]] struct S2 {...} s2var;` is attached to the VARIABLE
  (`attr ... on variable s2var @ L10:3`), not to the class; `struct
  [[cerastes::X3(3)]] S3` (after the class-key) is attached to the type
  (`attr ... on type S3 @ L11:10`, `syntactic_location: tag name`).
- This is the C++ rule: an attribute-specifier-seq before a simple-declaration
  appertains to the declared entities, and there are none for `S1` and `E4`.
- M2 consequence: class-level annotations must go after the class-key
  (`struct [[cerastes::...]] S`), never before the declaration.

**Sub-result 5c: attribute after the parameter list: PARTIAL, unusable as a
contract carrier**
- A single declaration keeps it, on the function's `tk_routine` TYPE
  (`source_corresp.attributes` of the type, `syntactic_location: post func`),
  not on the routine: plain `a2`/`p1`/`p2`, function template `p3`/`p4`, class
  member `c1`; the instance carries it on its own type (`p3<int>`, `c1` of
  `Box<int>::get`). Walker: `attr [[cerastes::p3(x > 0)]] on type @ L14:35`.
- Redeclaration loses it:
  - non-template `g`: `a2` at lines 26 and 28; only the line-28 one remains;
  - function template `tmpl`: `b2` at lines 42 and 45 (first declaration at 39
    has none): neither remains, in the prototype or the instance;
  - function template `s5` (`attr_postfunc.cpp`): declaration with `p5`, then
    definition with `p6`: only `p5` remains (on the prototype and the
    instance); `p6` is dropped.
  So for function templates only the attribute of the declaration that creates
  the function type survives; for `g` the later declaration won.

**What this means for M2**
- Prefix attributes on declarations and attributes on loop and null statements
  survive in prototypes and instances, so `invariant`, `check`, `fold`/`unfold`
  and prefix placements could ride the unrecognized-attribute path.
- `requires`/`ensures` go after the parameter list (5c). The unrecognized path
  cannot carry them reliably: a contract on a redeclaration is lost, and a
  contract on a template declaration plus definition keeps only one. M2 must
  register these attributes (plan.md's "check 5 fails" branch for this
  placement) so that EDG applies them to the entity at declaration time,
  instead of reading them from the IL afterwards. That registration is expected
  to be able to combine a declaration's and a definition's contracts, but this
  is NOT verified here: M2 must test it (contracts on a declaration plus a
  redeclaration, on a template declaration plus definition, and on a member
  defined outside its class).
- Warning #2803 appears for every unregistered `cerastes::` attribute. Once M2
  registers the namespace, #2803 disappears, so the drop of a prefix attribute
  on a declaration that declares nothing (5b-class: `[[cerastes::X1]] struct
  S1 {};`) becomes completely silent. M2 needs a must-fail test or a diagnostic
  for it.

### Gate G1 verdict: **PASS**, proceed to M2

Decided by the orchestrator from the sub-results above. The Opus
soundness-auditor reproduced the evidence and agreed in review passes 2 and 3.

| Check | Result | Regression test |
|---|---|---|
| 1. `v + 1` has `type_kind tk_template_param`; `(T)` is its own `eok_cast` | PASS | `g1/check1_inc.cpp.golden` |
| 2. A never-instantiated template has a prototype routine with a full body | PASS | `g1/check2_uninstantiated.cpp.golden`, `contrast/never_instantiated.cpp.golden` |
| 3. Instances carry `assoc_template` and deduced or explicit `template_arg_list` | PASS | `g1/check3_instances.cpp.golden` |
| 4. `std::integral` is reachable from all four constraint locations | PASS | `g1/check4_constraints.cpp.golden`, `contrast/not_integral.cpp.golden` |
| 5. Unrecognized `[[cerastes::…]]` attributes survive with namespace, name, raw tokens and positions | PASS for every placement plan.md lists (5a, 5a-redecl, 5b, 5b-null) | `g1/check5_attributes.cpp.golden`, `inventory/attr_*.golden` |

Checks 1 and 2 pass, so the Clang front-end fallback (the record plan.md
names `0003-frontend-fallback.md`) is not triggered and was not written.
Decision number 0003 went to the EDG fork branch instead.

Check 5 passes as specified:
- prefix on a function declaration, including across redeclarations;
- loop statements;
- null statements in every position tried;
- each of these plain, in a function template's prototype instantiation and
  in its instance.

The after-parameter-list placement (5c) is outside the list, and it is the
one M2 plans to use for `requires`/`ensures`. The unrecognized path does not
carry it reliably: it lands on the function type and is lost on
redeclaration. So for that placement M2 follows plan.md's "check 5 fails"
branch and registers the attributes explicitly. Registration is an EDG source
change, which needs a fork branch and an upstream PR before the pin can move
(invariant 5).

The walker prints a cross-check warning whenever a property behind checks 2
and 3 breaks:
- `warning: template … has no prototype routine`;
- `… has a definition but its prototype routine has no body`;
- `warning: instance … has no assoc_template`.

No golden contains any of these warnings, so a pin move that breaks check 2
or check 3 fails `scripts/check.sh`. The self-test hook
`--cerastes_walk_selftest` shows that the first and third warnings can
actually fire (`scripts/test_run_tests.py`, `WalkerSelfTestHookTests`). No
test exercises the second warning; the case it covers (pruned bodies) is
prevented outright, because `back_end()` refuses to run when EDG's
remove-unneeded-entities mode is on.

### Walker gaps

Fixed after the first pass (verified against `il_display`, Task 4):
- Operations nested in `tpck_expression` constants are now printed:
  `(T)100`, `!(a < b)` (`eok_not`, `eok_lt`), `++i` (`eok_pre_incr`), the
  default argument `(T)1`, `&r` initializers, and the member-selection
  operations of `&t->m_x` in `mirror`.
- `tconst` takes its position from the enclosing `enk_constant` node (for
  example `@ L15:27`). The remaining `@ -` lines (`op eok_lvalue` in `mirror`,
  and some `op eok_cast` and `tconst` lines in `contrast/local_statics.cpp`)
  are nodes that have no position in `il_display` either.
- Attributes on fields, classes and local variables are reported as top-level
  `attr ... on field Box<T>::v` / `on type Box<T>` / `on variable s2var` lines (owner labels are qualified)
  (`attr_classtmpl.cpp`, `attr_class_prefix.cpp`).
- Each output ends with `end walk: R routines reported, ...`; R equals the
  number of `routine` lines in every golden.
- Fixed after audit passes 2-3b:
  - lambda and local-class bodies are attributed to their own routine;
  - enumerator initializers and other functions' default arguments are no
    longer printed under the routines that use them;
  - the folded non-dependent operations written in a prototype body (`3 + 4`,
    `1 << 4`, `sizeof(T) * 2`) are printed;
  - initializers of constant-initialised local statics are walked;
  - constructor mem-initializers and default arguments are walked.

  Probes: `contrast/{lambda_bodies,enum_init_expr,folded_in_body,callee_default_arg,local_statics}.cpp`.

Still open:
- The `tpck_member` constants inside `&t->m_x` print `@ L54:12` / `@ L54:21`.
  `il_display` does not print those constant entries; the positions are the
  source columns of `m_x` and `m_y` (and equal the `enk_field` positions in
  the instances), but could not be read from the constant itself.
- The kind of a constant whose entry `il_display` does not print
  (`tpck_unknown_function`, `tpck_member`) cannot be cross-checked there.
- Walker order of `op` lines follows source/execution order (initializer,
  condition, body, increment), not tree pre-order of each statement.
- Expressions embedded in types are not walked. Examples: array bounds such as
  `int arr[G + 1]`, `sizeof(int[G * 2])`, C VLA bounds, and template-argument
  expressions inside types (`Holder<sizeof(T) / 2>`). M3 must decide how to
  treat them, especially VLA bounds in C.
- `static_assert` expressions in bodies are not walked, including dependent
  ones (`static_assert(sizeof(T) * 2 > 1)`).
- While the walker is inside a routine's own default argument, it may follow
  file-scope folds. It does this for the whole default-argument expression,
  so a callee's default argument nested inside it is followed too. Example:
  in `T f(T a, int c = h() + 1)` with `int h(int a = 1 << 2)`, `f`'s
  prototype lists `h`'s `eok_shiftl` at `h`'s position. The fix is to allow
  file-scope follows only for the top-level `default_arg_expr` constant. No
  golden exercises this (audit pass 3d).
- Default arguments that EDG has not evaluated (`default_arg_expr == NULL`)
  are not walked.

## Clang query → EDG IL

Columns: VeriFast site | Clang query | purpose there | EDG IL equivalent |
evidence | notes/gaps. Rows marked **(derived)** have no single IL field. The
evidence column cites the probe and the `il_display` fields seen; a cell that
says "derived" or "UNCLEAR" says why a probe did not show the fact. Rows whose
EDG field was read from `il_def.h` only (not shown by a probe) are marked
"derived" in the evidence cell.

Common IL conventions used below:

- *dep type*: a type involving a template parameter, i.e. a `tk_template_param`
  type (`il_def.h:5598`) or a pointer/reference/typeref/routine type built from
  one. `a_param_type.type_involves_template_param` (`il_def.h:6438`) is a
  ready-made flag for parameter types only.
- *skip typerefs*: `skip_typerefs(type)` (`il.h`, used throughout the EDG
  sources) is EDG's `getCanonicalType` (but it keeps cv-qualifiers on the
  result of `make_qualified_type`; check qualifiers separately).
- *routine body*: `il_header.function_def_table[routine->function_def_number].scope`
  (`mem_tables.h:81`, `il.rst:1737`), then `scope->variant.routine.parameters`
  and `scope->assoc_block` (`il_def.h:18401-18445`). The statements are
  `a_statement` (`il_def.h:16516`) linked by `next`.

### A. `collectIntegralTypeParams` (ASTSerializer.cpp:19-52) and `getIntegralTypeParams` (:642-658)

| # | VeriFast site | Clang query | Purpose | EDG IL equivalent | Evidence | Notes / gaps |
|---|---|---|---|---|---|---|
| A1 | :25 | `Expr::IgnoreParens()` | skip parentheses | `eok_parens` (`il_def.h:14021`) exists only if `PARENS_IN_IL`, which is 0 in this build (`--dump_configuration`; `targ_def.h:4742`), so parentheses are absent from the IL as nodes; the node flag `is_parenthesized` is set instead. | `check1_inc.cpp` `inc`: the add node prints `is_parenthesized: TRUE`; no `eok_parens` node anywhere in the dump (`PARENS_IN_IL 0`, `--dump_configuration`). The parenthesized state is a flag on the node (`an_expr_node`, il_display field `is_parenthesized`). | Parentheses are not an IL node here; the flag `is_parenthesized` keeps the information, so `a+(b*c)` and `a+b*c` differ only by tree shape and that flag. |
| A2 | :26-33 | `dyn_cast<BinaryOperator>`, `getOpcode() == BO_LAnd`, `getLHS/RHS` | walk conjuncts of a constraint | `an_expr_node.kind == enk_operation` (`il_def.h:15121`), `variant.operation.kind == eok_land` (`il_def.h:14153`), `variant.operation.operands` list: first operand, then `->next` (`il_def.h:15335`) | `check4_constraints.cpp` `f5` (`requires std::integral<T> && std::integral<U>`): requires-clause expr `kind: enk_operation`, `operation.kind: eok_land`, `operation.type_kind: tk_integer`, `do_not_interpret: TRUE`, operands = two `enk_concept_id` nodes. | |
| A3 | :34-36 | `dyn_cast<ConceptSpecializationExpr>` | find a concept-id | `an_expr_node.kind == enk_concept_id` (`il_def.h:13688`), `variant.concept_id` (`il_def.h:15710`) | `check4` `f1`: `kind: enk_concept_id`, `is_type_constraint: TRUE`, `concept_id.concept_template: template@...`, `concept_id.args: NULL`. `f2`/`f3`: `enk_concept_id`, `is_type_constraint` absent (FALSE). | A type-constraint is an `enk_concept_id` with `is_type_constraint` (`il_def.h:15095`) whose first argument is implicit: it is NOT in `args` (see the evidence). |
| A4 | :37-38 | `getNamedConcept()->getQualifiedNameAsString() != "std::integral"` | identify the concept | `variant.concept_id.concept_template` (an `a_template`, `kind == templk_concept`, `il_def.h:17274`); name = `source_corresp.name` (`il_def.h:3269`); qualification = walk `source_corresp.parent_scope` (`il_def.h:3297`) up to the `std` namespace scope. **(derived)** | `check4` `f1`: concept template `kind: templk_concept`, `name: "integral"`, `parent_scope` = `sck_namespace` whose `assoc_namespace` is `__1`, i.e. the libc++ inline namespace: the qualified name is `std::__1::integral`. | Compare name and namespace chain, not pointers: the concept template entry may be one of several `a_template` entries (use `canonical_template`, `il_def.h:17411`). |
| A5 | :41-45 | `getTemplateArguments()`, `size() == 1`, `args[0].getKind() == Type` | the concept's single type argument | `variant.concept_id.args` (list of `a_template_arg`, `il_def.h:15714`), `a_template_arg.kind == tak_type` (`il_def.h:7672`), `variant.type` | `check4` `f2`: `concept_id.args:` list with one entry `type: T#(1,1)`, `explicitly_specified: TRUE`; `f3` same. For the type-constraint forms (`f1`, `f4`) `concept_id.args: NULL`: the constrained parameter is implicit, not in `args`. | For a type-constraint `args` is NULL (the constrained parameter is implicit); for requires-clause and trailing forms `args` lists `T`. |
| A6 | :46-47 | `args[0].getAsType()->getAs<TemplateTypeParmType>()` | is the argument a plain type parameter | `skip_typerefs(arg->variant.type)->kind == tk_template_param` with `variant.template_param.kind == tptk_param` (`il_def.h:9289`, `10896`) | `check4` `f2`: arg type prints `T#(1,1)`; its entry has `kind: tk_template_param`, `kind: tptk_param`. | `tptk_member` / `tptk_unknown` are not plain parameters. |
| A7 | :48-50 | `param->getDepth() == depth`, `param->getIndex()` | which template parameter | `variant.template_param.extra_info->coordinates.{depth,position}` (`il_def.h:9377`, `4526-4540`); `position` is 1-based ("1 is first param"), Clang's index is 0-based | `check4` `f1`: the type `T#(1,1)` has `coordinates.position: 1`, `coordinates.depth: 1` (`T#(depth,position)` in type names). `check3` `pick<T,U>`: `U` prints `U#(1,2)`, so position is 1-based. | Off-by-one trap. Depth is the template nesting depth; compare with the depth of the function template's own `a_template_decl` (see P4). |
| A8 | :650-655 | `decl->getAssociatedConstraints(...)` | gather all constraints of the template | four locations: type-constraint of each type parameter (`extra_info->type_constraint`, `il_def.h:9386`), `template_decl->constraint.requires_clause->constraint` (`il_def.h:17234`, `12215`), `routine->trailing_requires_clause->constraint` (`il_def.h:13107`), and type-constraints of `is_abbreviated` parameters (`il_def.h:17168`). **(derived)**: union of these. | `check4`: all four locations seen. `f1`: `T` type entry `constraint.type_constraint: expr-node` (enk_concept_id). `f2`: `template-decl` requires clause -> `requires-clause@` -> `constraint:` enk_concept_id. `f3`: `routine` `trailing_requires_clause: requires-clause@` -> enk_concept_id. `f4`: template-parameter with `is_abbreviated: TRUE`, `ptr: type std::__1::integral auto`, with `type_constraint` enk_concept_id. `f5`: requires-clause `eok_land` of two concept ids. | The type-constraint of a `tptk_param` parameter is reached through `a_template_parameter.variant.type.ptr` → `variant.template_param.extra_info`. |
| A9 | :648-649, :653 | `getTemplateParameters()`, `->size()`, `->getDepth()` | parameter count and depth | count `a_template_decl.param_list` (`il_def.h:17222`; `a_template_parameter.next`, `il_def.h:17160`); depth: count `a_template_decl.parent` links (`il_def.h:17217`) or read from a parameter's coordinates **(derived)** | `check4` `f5`: `template-decl.param_list` -> `template-parameter@ T` -> next `U` (list via `next`). Depth from `coordinates.depth` of each parameter's type. `a_template_decl.parent` not exercised (no nested templates in the probes): derived. | |

### B. `IntegralTypeParams::indexOf` (:61-70), `isSupportedGenericType` (:76-97), `isBuiltinIntegerType` (:100-105)

| # | VeriFast site | Clang query | Purpose | EDG IL equivalent | Evidence | Notes / gaps |
|---|---|---|---|---|---|---|
| B1 | :63-64 | `type.getCanonicalType().getTypePtr()`, `dyn_cast<TemplateTypeParmType>` | is this type a template parameter | `skip_typerefs(type)->kind == tk_template_param` | `check1` `inc`: `v` is `enk_variable` with `type: const T#(1,1)`; its unqualified type is `tk_template_param`/`tptk_param` (`kind: tptk_param`). | cv-qualifiers (`const T v`) are on the type entry; check the unqualified type. |
| B2 | :65-67 | `getDepth()`, `getIndex()`, `params.test(index)` | match against integral set | coordinates as in A7 | as A7. | same off-by-one |
| B3 | :79 | `QualType::isDependentType()` | does the type involve a template parameter | **(derived)** recursive walk: `tk_template_param`; `tk_pointer` → `variant.pointer.type`; `tk_routine` → return and parameter types; `tk_array`; typeref → `skip_typerefs`; class types that are nonreal (`is_nonreal`, `il_def.h:10807`). For parameter types, `a_param_type.type_involves_template_param`. | derived. Evidence of the pieces: `T#(1,1) *` is `tk_pointer` (`ops.cpp` `deref`, variable `p`), `T#(1,1) &` is `tk_pointer` (variable `r`), and `T#(1,1) (T#(1,1))` is the routine type (`check2` `never_used`). `param-type` entries carry `type_involves_template_param: TRUE` (`check5` Box::get param `x`). | No `isDependentType` flag on `a_type`. |
| B4 | :83-85 | `dyn_cast<TemplateTypeParmType>`, `getIdentifier()` | has a name | name lives on the `a_template_parameter` (`source_corresp.name`), reached from `a_template_decl.param_list` by coordinates; the type itself does not carry it **(derived)** | `check4` `f1`: `template-parameter@` has `source_corresp.name: "T"`. `f4` (abbreviated): `is_abbreviated: TRUE`, no `name:` line at all, so a name test rejects abbreviated parameters unless `is_abbreviated` is handled. | Abbreviated `auto` parameters are `is_abbreviated`; their generated names must be checked. |
| B5 | :85 | `isParameterPack()` | variadic pack | `variant.template_param.is_pack` (`il_def.h:10903`) or `a_template_parameter.is_pack` (`il_def.h:17166`) | derived (not probed: no pack in the probes); `is_pack` fields are in the block layout (`il_def.h:10903`, `17166`). `check4` `f1` shows `is_pack` absent (FALSE). | |
| B6 | :88-91 | `dyn_cast<PointerType>`, `getPointeeType()` | pointer, recurse | `kind == tk_pointer` with `variant.pointer.is_reference == FALSE` (`il_def.h:10198`), pointee `variant.pointer.type` (`il_def.h:10231`) | `ops.cpp` `deref`: variable `p` has `type: ...: T#(1,1) *`; in the `eok_indirect` operand `p` the same type, whose `a_type.kind` is `tk_pointer`; the result `type: ...: T#(1,1)` has `template_param kind: tptk_param`. (Notation: see Setup, `type[kind]`.) | `tk_pointer` is also used for references (`il_def.h:5588`). |
| B7 | :92-95 | `dyn_cast<LValueReferenceType>` | lvalue reference, recurse | `kind == tk_pointer`, `variant.pointer.is_reference == TRUE`, rvalue-ness flag (`is_rvalue_reference`, `il_def.h:10203`) | `ops.cpp` `deref`: variable `r` `type: T#(1,1) &`, `is_parameter: TRUE`; `verifast-plain/generic_body.cpp` `assign`: `r` used through `eok_ref_indirect` (`type_kind: tk_pointer`). | VeriFast rejects rvalue references and everything else by returning false. |
| B8 | :101-104 | `dyn_cast<BuiltinType>`, `isInteger()`, `getKind() != Bool` | builtin integer other than bool | `skip_typerefs(t)->kind == tk_integer` (`il_def.h:5577`) with `variant.integer.bool_type == FALSE` (`il_def.h:10104`) **and** `enum_type == FALSE` (`il_def.h:10069`); `variant.integer.int_kind` (`il_def.h:10047`, `5646`) | `check3` `inc<unsigned char>`, `inc<long>`: `template_arg_list` entries `unsigned char`, `long` (type entries with `kind: tk_integer`). `integral.cpp` dump: `bool` is `kind: tk_integer`, `int_kind: char`, `bool_type: TRUE` (so `int_kind` alone cannot tell `bool` from `char`); `char32_t` is `kind: tk_integer`, `int_kind: unsigned int`, `char32_t_type: TRUE`. No enum was probed: the `enum_type` flag (`il_def.h:10069`) is read from the layout only. | `tk_integer` includes enums (`il_def.h:5577-5578`); Clang's `BuiltinType` does not. Excluding `bool` needs the `bool_type` flag, not `int_kind`; `char32_t` is `tk_integer` with its own flag and an `int_kind` shared with `unsigned int`. Arguments often arrive as `tk_typeref` (see F5): skip typerefs first. |

### C. `classifyIntegral`, `getOperandsParam` (:140-257)

| # | VeriFast site | Clang query | Purpose | EDG IL equivalent | Evidence | Notes / gaps |
|---|---|---|---|---|---|---|
| C1 | :163, :197, :273, :314 | `Expr::isTypeDependent()` | expression depends on a template parameter | **(derived)** `skip_typerefs(node->type)->kind == tk_template_param` (the node's own type, `il_def.h:14907`); for `enk_operation` also `variant.operation.type_kind == tk_template_param` (`il_def.h:15131`) | `check1` `inc`: `eok_add` node `type: <unnamed@...>` (`tk_template_param`, `kind: tptk_unknown`), `operation.type_kind: tk_template_param`; `v` has `type: const T#(1,1)`. The node `type` is `tptk_unknown`, not `T`, for an operation; the operand `v` has the real parameter type. | `type_kind` "may be `tk_unknown`" for dependent operations (`il_def.h:15137`); check 1 decides whether the node `type` is the reliable signal. |
| C2 | :314 | `Expr::isInstantiationDependent()` | dependent in any way, including value-dependent | **(derived)**: node type dependent, or any operand dependent, or constant `kind == ck_template_param` (`il_def.h:5364`, `3659`), or a `sizeof`/`alignof` of a dependent type (`enk_sizeof`, `il_def.h:15473`) | derived. Seen: dependent operations carry `type_kind: tk_template_param` (`check1`) except `eok_question` and a cast inside a `tpck_expression` wrapper (`type_kind: tk_unknown`, `ops.cpp` `cond`, `check1` `const_cast_c`), and some wrapped nodes use `type_kind: tk_integer` (`ops.cpp` `logic`'s `eok_land`). `type_kind` alone is not a reliable dependence test; use the node `type` plus a walk over operands. | No flag; needs a recursive walk over operands. |
| C3 | :164, :272 | `getType()->isBooleanType()` | boolean-typed expression | `skip_typerefs(node->type)->kind == tk_integer && variant.integer.bool_type` (`il_def.h:10104`) | `check1` `less10`: the `eok_lt` node prints `type: ...: <unnamed@...>` (a `tk_template_param`, `kind: tptk_unknown`), not `bool`; in the instance `less10<int>` it is `type: ...: bool`. `check2` `used`: the `while` condition is wrapped in an `eok_bool_cast` node of `type: ...: bool` with `operation.type_kind: tk_template_param`. So a dependent comparison is NOT typed `bool` in the prototype; only the implicit condition conversion node is. | Settled: a dependent comparison has type `tptk_unknown`, not `bool`; `bool` appears on the `eok_bool_cast` wrapper of a condition. |
| C4 | :169-173 | `dyn_cast<IntegerLiteral>`, `getType()->isSpecificBuiltinType(Int)`, `getValue().ule(32767)` | small int literal | `enk_constant` (`il_def.h:15339`) → `constant.ptr->kind == ck_integer` (`il_def.h:3622`), value `variant.integer_value` (`il_def.h:5035`), type `int` (`int_kind == ik_int`) | `check1` `inc`: `enk_constant`, `constant: ck_integer`, `integer_value: 1`, `type: int` (il_display `constant@...: kind: ck_integer`). | `integer_value` prints as a plain decimal (`integer_value: 1`). |
| C5 | :178-187 | `dyn_cast<DeclRefExpr>`, `isa<VarDecl>(getDecl())` | reference to a variable (not function/enumerator) | `enk_variable` (`il_def.h:15351`, `variable.ptr`); function refs are `enk_routine` (`il_def.h:15362`); enumerators are `enk_constant` | `check1` `inc`: `v` is `enk_variable`, `variable: ... v`, `type: const T#(1,1)`, `is_lvalue: TRUE`. | `enk_param_ref` (`il_def.h:13662`) is only used in function-type signatures, not bodies. |
| C6 | :190-209 | `isa<CStyleCastExpr, CXXStaticCastExpr>`, `CastExpr::getSubExpr()` | explicit cast to `T` | `enk_operation`, `eok_cast` (`il_def.h:13870`), single operand; result type = node `type`. Source cast spelling (C-style vs `static_cast`) is not a distinct operator; a source-position/range check is the only difference **(derived)** | `check1` `inc`: `eok_cast` (`type: T#(1,1)`, `operation.type_kind: tk_template_param`) with operand the `eok_add` node, i.e. the cast is its own node. `static_cast_v`: also `eok_cast`, same shape. `const_cast_c` `(T)100`: NOT an `eok_cast` at top level: `enk_constant`, `ck_template_param`, `kind: tpck_expression`, `type: T#(1,1)`, whose `expr:` is an `enk_operation eok_cast` with `type_kind: tk_unknown` over `ck_integer 100`. | The `(T)100` form may fold into a `ck_template_param` constant (see check 1). An implicit conversion is also `eok_cast` with `compiler_generated` (`il_def.h:15107`). |
| C7 | :210-246 | `BinaryOperator` opcodes `BO_Add/Sub/Mul/Div/Rem` | arithmetic | `eok_add`, `eok_subtract`, `eok_multiply`, `eok_divide`, `eok_remainder` (`il_def.h:14049`, `14052`, `14056`, `14059`, `14062`) | `check1` `inc`: `eok_add`; `sum2`: `eok_add` on two `enk_variable`; `verifast-plain/integral.cpp` `uninstantiated`: nested `eok_add`. `eok_subtract/multiply/divide/remainder` not probed (same operator family): derived. | In dependent code EDG cannot apply integral promotions; the result type stays dependent. |
| C8 | :223-232 | `BO_LT/GT/LE/GE/EQ/NE` | comparison | `eok_lt`, `eok_gt`, `eok_le`, `eok_ge`, `eok_eq`, `eok_ne` (`il_def.h:14093`, `14092`, `14095`, `14094`, `14090`, `14091`) | `check1` `less10`: `eok_lt` (`type_kind: tk_template_param`); `ops.cpp` `logic`: `eok_eq`, `eok_gt`. `ne/le/ge` not probed: derived. | |
| C9 | :233-242 | `BO_LAnd/BO_LOr` | logical | `eok_land`, `eok_lor` (`il_def.h:14153`, `14156`) | `ops.cpp` `logic`, prototype: `eok_land` (`type_kind: tk_integer`, type tptk_unknown) over `eok_lor` (`parenthesized`, operands `eok_eq`, `eok_gt`). Instance `logic<int>`: same shape with `bool[tk_integer]` types. | `&&` and `||` in a body are covered by `ops.cpp` `logic` (prototype and instance). |
| C10 | :247-255 | `UnaryOperator`, `UO_LNot` | logical not | `eok_not` (`il_def.h:14025`) | `ops.cpp` `logic`, prototype: `!(a < b)` is an `enk_constant` whose constant is `ck_template_param`/`tpck_expression` (type `tptk_unknown`), whose `expr:` is `eok_not` (`operation.type_kind: tk_template_param`) over `eok_lt`; the instance has a bare `eok_not` of `type: ...: bool`. Walker (golden `ops.cpp.golden`): `tconst tpck_expression ... @ L7:58`, `op eok_not ... @ L7:58`, `op eok_lt ... @ L7:62`. The checker must see through a `tpck_expression` constant wrapper. | `!` is covered by `ops.cpp` `logic`; inside a prototype it sits in a `tpck_expression` wrapper. |
| C11 | :144-156 | (no new query; literal/param operand matching) | operands of an operator must be same param or literal | operand list walk as in A2 | n/a (no new query). | |

### D. `checkNoConversion`, `checkCondition`, and the visitor rules (:260-396)

| # | VeriFast site | Clang query | Purpose | EDG IL equivalent | Evidence | Notes / gaps |
|---|---|---|---|---|---|---|
| D1 | :264-265 | `getCanonicalType().getUnqualifiedType()` equality | types identical | `f_identical_types(t1, t2)` (`types.h:1185`) on `skip_typerefs`, ignoring qualifiers **(derived)**; for dependent types identity of `tk_template_param` entries is by `coordinates` | derived. `check3`: T appears as separate `type@` entries (`T#(1,1)` at several addresses, e.g. in `template_arg_list` and in the routine type), so compare `coordinates`, not pointers. `f_identical_types` is declared at `types.h:1185` (not run). | EDG may create several `a_type` entries for one parameter; compare coordinates, not pointers. |
| D2 | :282-288 | `getCond()` of `if`/`while`/`do`/`for`/`switch`, `?:` | condition expression | `a_statement.expr` (`il_def.h:16586`) for `stmk_if`, `stmk_while`, `stmk_end_test_while`, `stmk_for`, `stmk_switch`; the `?:` condition is the first operand of `eok_question` (`il_def.h:14237`) | `check2` `never_used`: `stmk_for` expr = `eok_bool_cast` (bool) over `eok_lt`; inner `stmk_if` same. `used`: `stmk_while` same. `ops.cpp` `cond`: `eok_question` operand 1 = `eok_bool_cast` over `eok_lt`, then the two branches. | `stmk_if`'s two branches are `if_stmt.then/else_statement` (`il_def.h:16620-16636`). |
| D3 | :296 | `FunctionDecl::getReturnType()` | return type | `skip_typerefs(routine->type)->variant.routine.return_type` (`il_def.h:10235`) | `check2` `never_used`: routine `type: T#(1,1) (T#(1,1))` (return type first, then parameter types); the `tk_routine` type shows `return_type`. | |
| D4 | :299 | `FunctionDecl::parameters()`, `ParmVarDecl::getType()` | parameter types | type view: `…->variant.routine.extra_info->param_type_list` (`il_def.h:7396`), linked `a_param_type.next` (`il_def.h:6385`); variable view: routine-body scope `variant.routine.parameters` (`il_def.h:18405`) | `check5` `Box<T>::get` routine type: `param_type_list: param-type@ ... name "x" type T#(1,1) type_involves_template_param: TRUE`; the function scope has `parameters: variable@ x` (`check1` `inc` scope: `parameters: variable@ v`). | `a_param_type.type` has top-level qualifiers removed (`il_def.h:6392`); `declared_type` keeps the declared form. |
| D5 | :301-303 | `hasDefaultArg`, `hasUnparsedDefaultArg`, `hasUninstantiatedDefaultArg`, `getDefaultArg()->isInstantiationDependent()` | dependent default argument | `a_param_type.has_default_arg`, `has_unevaluated_template_default` (`il_def.h:6414`, `6424`), `default_arg_expr` (`il_def.h:6519`), then C2 | `ops.cpp` `dflt`: `param-type` for `b` has `has_default_arg: TRUE`, `has_unevaluated_template_default: TRUE`, `type_involves_template_param: TRUE`, `default_arg_expr:` an `enk_constant` whose constant prints `((T#(1,1))1)` (`ck_template_param`). Instance `dflt<int>`: `has_default_arg: TRUE`, default constant `1` (int). | |
| D6 | :307-308 | `getBody()`, `TraverseStmt` | walk the body | routine body (see conventions), then `traverse_statement` / `traverse_expr` (`il_walk.h:452-461`) | `check1`..`check4`, `check2` `never_used`: routine `function_def_number: 32` -> `Intermediate language for memory region 33 (function "inc")` containing `func-scope scope@` `kind: sck_function`, `routine.ptr: ... inc`, `parameters: ...`, `assoc_block: statement@` (`stmk_block`, `statements:` chain via `next`). The body is in a separate memory region per function; both prototype and instance have one. | |
| D7 | :317-319 | `dyn_cast<DeclRefExpr>`, non-`VarDecl` | any reference that is not a variable | `enk_routine` (function), `enk_constant` (enumerator), `enk_field` as operand of member selection | `check3` `twice_inc`: a dependent callee is an `enk_constant` (unresolved name), not `enk_routine` (F9). References to variables are `enk_variable` (C5). Function and enumerator references were not probed: derived. | |
| D8 | :320-322 | `isa<ParenExpr, ImplicitCastExpr, ConditionalOperator>` | harmless nodes | `eok_parens` (see A1), `eok_cast` with `compiler_generated` (`il_def.h:15107`), `eok_question` (`il_def.h:14237`) | parens: A1. Implicit casts: the `eok_bool_cast` condition wrapper (`check2`, `ops.cpp` `cond`). `eok_question`: `ops.cpp` `cond`, `type_kind: tk_unknown`, type tptk_unknown in the prototype, `int` in the instance. | |
| D9 | :324-331 | `UnaryOperator`, `UO_AddrOf`, `UO_Deref` on non-dependent operand or non-integral | address-of / dereference | `eok_address_of`, `eok_indirect` (`il_def.h:13857`, `13866`); references: `eok_reference_to`, `eok_ref_indirect` (`il_def.h:13858`, `13867`) | `ops.cpp` `deref`: `*p` is `eok_indirect` (`operation.type_kind: tk_pointer`, result type `T#(1,1)`); `T &r` used in `*q` and `verifast-plain/generic_body.cpp` `assign`: `eok_ref_indirect`. `T *q = &r;`: `init_kind: initk_dynamic`, `dynamic-init` `kind: dik_constant` whose constant (`ck_template_param`/`tpck_expression`, prints as `((T#(1,1) *)(& (ref-* r)))`) has `expr:` `eok_cast` (`operation.type_kind: tk_unknown`, 15124:52) over an `enk_constant` (15124:52) that is another `tpck_expression` whose `expr:` is `eok_address_of` (`operation.type_kind: tk_pointer`, 15124:52) over `eok_ref_indirect` (15124:53) over `enk_variable r`. Walker (golden `ops.cpp.golden`): `op eok_cast type_kind=tk_unknown type=T * @ L13:52`, `tconst tpck_expression ... @ L13:52`, `op eok_address_of type_kind=tk_pointer ... @ L13:52`, `op eok_ref_indirect type_kind=tk_pointer type=T @ L13:53`. | `&r` IS an `eok_address_of` node, but two `tpck_expression` constants deep and under an `eok_cast tk_unknown`. `T&` parameters are used through `eok_ref_indirect` (`ops.cpp` `deref`, `generic_body.cpp` `assign`). |
| D10 | :332-337, :343-348 | `BinaryOperator` with `BO_Assign`; `getLHS()->getType()`, `getRHS()` | assignment | `eok_assign` (`il_def.h:14113`), operands in order | `verifast-plain/generic_body.cpp` `copy_through_locals`: `z = y;` is `stmk_expr` over `eok_assign` (`type_kind: tk_template_param`, type tptk_unknown), operands `enk_variable z`, `enk_variable y`. `assign`: lhs `eok_ref_indirect` over `enk_variable r`. | Compound assignments are separate operators (`eok_add_assign`, `il_def.h:14124`); VeriFast rejects them (falls to `fail()`). |
| D11 | :350-354 | `ConditionalOperator`, `getCond/getTrueExpr/getFalseExpr` | ternary | `eok_question`: operands cond, then, else (`il_def.h:14237`) | `ops.cpp` `cond`: `eok_question` (`type_kind: tk_unknown`; operands: condition `eok_bool_cast`, then `a`, else `b`). | `?:` over `T` is covered by `ops.cpp` `cond`. |
| D12 | :356-369 | `VarDecl::getType()`, `hasInit()`, `getInitStyle()`, `getInit()` | local variable and initializer | variable `a_variable` in the function scope's `variables` list (`il_def.h:11210`, `type` `il_def.h:11221`); initialization is a `stmk_init` statement carrying a `a_dynamic_init` (`il_def.h:3957`); init style: `has_explicit_initializer`, `has_parenthesized_initializer`, `has_direct_braced_initializer` (`il_def.h:11617-11625`) **(derived)** | `verifast-plain/generic_body.cpp` `copy_through_locals`: `T y = x;` = `stmk_decl` followed by `stmk_init`; `T z;` = `stmk_decl` only. `ops.cpp` `deref` `T *q = &r;`: variable `init_kind: initk_dynamic`, `dynamic: dynamic-init@ kind: dik_constant`. `check1` `plus_local`: `auto w = v + 1;` has `type: auto` (not resolved in the prototype). | Clang's CInit (`T y = x`) is "has_explicit_initializer && !parenthesized && !braced". |
| D13 | :371-376 | `ReturnStmt::getRetValue()` | return value | `stmk_return` (`il_def.h:15814`), `a_statement.expr` | `check1` `inc`: `stmk_return` with `expr:` = the cast node (tree: `stmk_return` -> `eok_cast`). | |
| D14 | :382-396 | `IfStmt`, `WhileStmt`, `DoStmt`, `ForStmt`, `SwitchStmt` | control-flow statements | `stmk_if`, `stmk_while`, `stmk_end_test_while` (do-while), `stmk_for`, `stmk_switch` (`il_def.h:15802-15870`); for-loop extras in `a_for_loop` (`il_def.h:15911`) | `check2` `never_used`: `stmk_for` (with `a_for_loop`), `stmk_if`, `stmk_while`, `stmk_block`; `stmk_end_test_while`/`stmk_switch` not probed: derived. | `[[cerastes::invariant]]` attaches to these statements' `attributes`. |
| D15 | :338-340 | catch-all: member of type parameter, dependent call, casts, `sizeof(T)`, `new T` | everything else is rejected | `eok_dot_field` / `eok_points_to_field` (`il_def.h:14174`, `14178`), `eok_call` (`il_def.h:14248`), `enk_sizeof` (`il_def.h:15473`), `enk_new_delete` (`il_def.h:15421`), `ck_template_param` constants of kinds `tpck_member`, `tpck_unknown_function`, `tpck_sizeof` (`il_def.h:4414-4437`) | `check1` `const_cast_c` shows a `ck_template_param`/`tpck_expression` wrapper; `eok_dot_field`, `eok_call`, `enk_sizeof`, `enk_new_delete` not probed (the checker whitelists, so unseen kinds reject): derived. | Whitelist, fail closed: any node kind not in the accepted set rejects. |

### E. `canBeVerifiedGenerically` (:399-419), `isVerifiedGenerically` (:625-640), name functions

| # | VeriFast site | Clang query | Purpose | EDG IL equivalent | Evidence | Notes / gaps |
|---|---|---|---|---|---|---|
| E1 | :401 | `getTemplatedDecl()` | the function inside a template | `a_template.prototype_instantiation.routine` (`il_def.h:17391`) for `kind == templk_function` | `check2` `never_used`: `template@ kind: templk_function`, `routine: routine@ never_used`, routine `is_prototype_instantiation: TRUE`, `defined: TRUE`, `function_def_number: 32`. | |
| E2 | :402 | `isa<CXXMethodDecl>(funcDecl)` | reject member functions | `routine->source_corresp.is_class_member` (`il_def.h:3404`) or the template `kind == templk_member_function` (`il_def.h:17263`) | `check5` `Box<T>::get`: `routine` `is_class_member: TRUE` (`source_corresp`) and its type has `has_this_param: TRUE`, `this_class: Box<T#(1,1)>`. | |
| E3 | :405-411 | iterate `getTemplateParameters()`, `dyn_cast<TemplateTypeParmDecl>`, `isParameterPack`, `getName().empty()` | all parameters are named, non-pack type parameters | `a_template_decl.param_list` (`il_def.h:17222`) → `a_template_parameter.kind == tpk_type` (`il_def.h:17144`), `is_pack` (`il_def.h:17166`), `source_corresp.name != NULL` (`il_def.h:3269`); `tpk_nontype` / `tpk_template` reject | `check3` `pick<T,U>`: two `template-parameter@` `kind: tpk_type`; `check4` `f4`: abbreviated parameter has no name (see B4). Non-type / template-template parameters not probed: derived (`tpk_nontype`, `tpk_template`). | |
| E4 | :413-415 | `getDefinition()` | use the defining declaration | `a_template.definition_template` on the canonical entry (`il_def.h:17416`); its `prototype_instantiation.routine` has the body | `generic_body.cpp` `declared_first` (declared line 34, defined line 39): two `template@` entries, both `templk_function`; first (`decl_position.seq: 34`) has `canonical_template` = itself and `definition_template` = the second entry; the second has `definition_template: NULL`, `canonical_template` = first. Both point `routine:` at the same prototype routine. | One `a_template` entry per declaration; the canonical one points to the definition (`il.rst:3070-3075`). |
| E5 | :417 | `getTemplateParameters()->getDepth()` | nesting depth | see A9 | as A9 (derived). | |
| E6 | :628 | `getCanonicalDecl()` | one template per redeclaration set | `a_template.canonical_template` (`il_def.h:17411`) | as E4: `canonical_template` on both entries; the prototype and the instances have `assoc_template` = the canonical entry (`routine@...: assoc_template: template@7b5aa63650`). | |
| E7 | :632-635 | `specializations().empty()` | "if instantiated" fallback | **(derived)** no list in the IL: scan all routines for `assoc_template` whose `canonical_template` equals the template and `template_arg_list != NULL` (`il_def.h:13166`, `13176`) | `check3`: instances are ordinary routines (`inc<long>`, `inc<unsigned char>`, `pick<int,char>`, `twice_inc<short>`, `inc<short>`) with `is_template_function: TRUE`, `assoc_template: template@` and `template_arg_list`; they are found only by scanning the routine list; the template entry has no instance list. | Front-end `instantiations` list is symbol data (`templates.rst:436`), not available to a back end. |
| E8 | :561-600, :580-623 | `printQualifiedName`, `getOriginalType().print` | stable name for a template | qualified name from `source_corresp.name` + `parent_scope` chain (`il_def.h:3297`); parameter types via `il_to_str` (`il_to_str.h`) **(derived)** | derived (names via `name:` and `parent_scope`; seen in `check4`: concept in `std::__1`). | Not needed for the IL check itself; needed for the Viper name. |
| E9 | :614 | `integral.test(i)` printed as `std::integral T` | name embeds integral params | no IL field; follows from A8 | n/a. | |
| E10 | (all rows) | `RecursiveASTVisitor` | traversal | `traverse_expr`, `traverse_statement` (`il_walk.h:452-461`) | The walker (`src/backend/walker.cpp`) walks bodies with `traverse_statement` / `traverse_expr` (`il_walk.h:452-461`). My cross-check did its own descent over `statements`/`next`/`operands`/`increment`/dynamic inits and got the same multiset of `op`/`tconst` lines for every prototype; I did not run `traverse_*` myself. | |

### F. `usesGenericProof` (:676-702) and the call-site code (`ExprSerializer.cpp:340-396`)

| # | VeriFast site | Clang query | Purpose | EDG IL equivalent | Evidence | Notes / gaps |
|---|---|---|---|---|---|---|
| F1 | ASTSerializer :677 | `getPrimaryTemplate()` | which template an instance came from | `routine->assoc_template` (`il_def.h:13176`) → `canonical_template` | `check3` `inc<long>`: `assoc_template: template@...f10`, same template as the prototype `inc` (`is_prototype_instantiation: TRUE`), `template_arg_list: type: long`. The prototype routine ALSO has `assoc_template` and a `template_arg_list` (`type: T#(1,1)`), so a routine is an instance only if `is_prototype_instantiation` is absent. | For member functions of class templates `template_arg_list` is NULL but `assoc_template` may still be set. |
| F2 | :679-680 | `getTemplateSpecializationKind() == TSK_ImplicitInstantiation` | not an explicit specialization or instantiation | `!routine->is_specialized` (`il_def.h:12533`) && `!routine->explicit_instantiation` (`il_def.h:12565`) && `is_template_function` (`il_def.h:12523`) **(derived)** | `specializations.cpp`: implicit `ident<int>` / `twice<int>`: `is_template_function: TRUE`, no `is_specialized`, no `explicit_instantiation`. `ident<char>` (explicit specialization): `is_specialized: TRUE`, `expl_template_arg_list_used: TRUE`. `twice<long>` (explicit instantiation): `explicit_instantiation: TRUE`. | Covered by `specializations.cpp`: `is_specialized` and `explicit_instantiation` distinguish the three cases. |
| F3 | :686-687 | `getTemplateSpecializationArgs()->asArray()` | the deduced/explicit arguments | `routine->template_arg_list` (list of `a_template_arg`, `il_def.h:13166`, `7683`) | `check3` `pick<int>('x')`: `template_arg_list:` entries `type: int` with `explicitly_specified: TRUE`, then `type: char` (deduced, no flag). Routine also shows `expl_template_arg_list_used: TRUE`. | `explicitly_specified` (`il_def.h:7689`) tells deduced from explicit. |
| F4 | :688 | `arg.getKind() != TemplateArgument::Type` | only type arguments | `a_template_arg.kind == tak_type` (`il_def.h:7672`) | `check3`: every `template_arg_list` entry is a `type:` (`tak_type`); non-type arguments not probed: derived. | |
| F5 | :691 | `arg.getAsType().getCanonicalType()` | canonical argument type | `skip_typerefs(arg->variant.type)` | `check3` `inc<unsigned char>`: the `template_arg_list` entry prints `type: ...: unsigned char`. BUT `verifast-plain/integral.cpp` `count_to<uint16_t>`: the entry is `type: ...: uint16_t`, a type entry with `kind: tk_typeref`, `typeref_type: ...: unsigned short`; the golden prints `template count_to args <T=uint16_t>`. (By contrast, `twice<char32_t>`'s entry is a plain `tk_integer`, `int_kind: unsigned int`, `char32_t_type: TRUE`: whether a typeref appears depends on how the argument was spelled.) | Typerefs DO occur in `template_arg_list`. M5 must skip typerefs (`skip_typerefs`) before deciding that an argument is a builtin integer type; classifying the entry's own `kind` would reject `uint16_t` arguments. |
| F6 | :692-695 | `isBuiltinIntegerType` on integral params | builtin integer, not bool/enum | as B8 | as B8. | |
| F7 | :696-699 | `isArithmeticType()`, `isEnumeralType()`, `isPointerType()` | scalar argument for non-integral params | `tk_integer` (incl. bool, enum), `tk_float` / `tk_complex` etc. (`il_def.h:5577`), `tk_pointer` with `!is_reference` **(derived)** | derived (only integer arguments probed). | Clang's `isArithmeticType` excludes complex and scoped enums; mirror precisely in M4. |
| F8 | ExprSerializer :344-347 | `CallExpr::isTypeDependent()`, `any_of(arguments(), isTypeDependent)` | dependent call is rejected | `eok_call` (`il_def.h:14248`), operand list; C1 on each operand | `check3` `twice_inc` prototype body `return inc(inc(v));`: both calls are `enk_operation eok_call`, `operation.type_kind: tk_unknown`, `type: <unnamed> [tptk_unknown]` (position 15127:36 and 15127:40). A dependent call is recognizable as `eok_call` with a dependent argument or an unresolved callee (F9). | In EDG dependent calls may have `type_kind == tk_unknown` and an unresolved callee (`tpck_unknown_function`). |
| F9 | ExprSerializer :350 | `getCallee()` | callee | first operand of `eok_call` (`enk_routine` node, `il_def.h:15362`) | `check3` `twice_inc`: first operand is `enk_constant` with `constant: constant@...: inc` (a `ck_template_param` constant, displayed by name only, `type: <unnamed> [tptk_unknown]`), not an `enk_routine`. A resolved non-dependent call would carry `enk_routine` (not probed). | |
| F10 | ExprSerializer :352-358 | `arguments()` | argument list | remaining operands of `eok_call` | `check3` `twice_inc`: after the callee, operand 2 of the inner call is `enk_variable v` (`const T#(1,1)`, `is_lvalue: TRUE`); for the outer call it is the inner `eok_call`. | |
| F11 | :367-373 | `DeclRefExpr::getDecl()` is a `FunctionDecl`; `usesGenericProof(func)` | call to an instance → generic proof | `enk_routine` operand with `routine.ptr` = instance routine (`il_def.h:15362`), then F1-F7 | `check3`: no `inc<T>` instance arises from the prototype body of `twice_inc` (the dependent call stays an unresolved `eok_call`); `inc<short>` appears only as the instance created from `twice_inc<short>`. A call to an instance is therefore only visible in instance bodies. | Inside a prototype body, calls to templates with dependent arguments cannot be resolved (D15). |
| F12 | :384-395 | `getPrimaryTemplate()`, `getTemplateSpecializationArgs()`, `targ.getAsType()` | build the generic-proof reference | F1, F3, F5 | as F1, F3, F5. | |

### G. Specialization iteration (`DeclSerializer.cpp:494-571`)

| # | VeriFast site | Clang query | Purpose | EDG IL equivalent | Evidence | Notes / gaps |
|---|---|---|---|---|---|---|
| G1 | :501, :505 | `getTemplatedDecl()`, `getQualifiedFuncName` | the template's function and name | E1, E8 | as E1. | |
| G2 | :509-522 | `getTemplateParameters()`, `getName()`, `integral.test(i)` | emit the parameter list | `param_list` walk: A9, E3, A8 | as A9/E3. | |
| G3 | :528-534 | `decl->isImplicit()` | implicit templates (deduction guides) have no contract | `source_corresp.is_generated`-style flag, or a compiler-generated template; not found at this reading | not probed: UNCLEAR (no implicit templates in the probes). | Gap: confirm in `il_def.h` whether `a_template` marks implicit templates; M5 only cares about function templates, so low risk. |
| G4 | :538-544 | `getDefinition()`, `getDescribedFunctionTemplate()`, `owner != decl` | serialize specializations once per redeclaration set | `definition_template` / `canonical_template` (E4, E6): only act on the canonical entry | as E4. | |
| G5 | :547 | `decl->specializations()` | all instances | no list: scan routines (E7) | as E7. | |
| G6 | :549-553 | `isExplicitInstantiationOrSpecialization()` | reject explicit instantiation and specialization | `routine->is_specialized`, `explicit_instantiation`, `specialized_with_old_syntax` (`il_def.h:12533`, `12565`, `12542`) | as F2. | Covered by `specializations.cpp` (see F2). |
| G7 | :554 | `usesGenericProof(spec)` | split instances | F1-F7 | as F1-F7. | |
| G8 | :566 | `spec->getSourceRange()` | position of an instance | `routine->source_corresp.decl_position` (`il_def.h:3327`), `a_template.definition_range` (`il_def.h:17359`) | `generic_body.cpp`: routine `decl_position.seq: 40` (the declarator of the definition), template entries `decl_position.seq: 34` and `39`. `EXTRA_SOURCE_POSITIONS_IN_IL` ranges (`expr_range`, `identifier_range`, `declarator_range`) are printed for every entry. | `EXTRA_SOURCE_POSITIONS_IN_IL` provides ranges. |
| G9 | :567 | `serializeFunctionDecl(spec)` | the instance body | instance routine body (conventions above); instance routines are real IL (not prototype) | `check3`: every instance has `function_def_number` (a body) because `tim_used` instantiates what is used. An unused template has no instance routine at all (`never_used` in `check2` has only its prototype). | An instance without a body (declared only, not required) has `function_def_number == NULL_function_def_number`. |

Total rows: 74 (A: 9, B: 8, C: 11, D: 15, E: 10, F: 12, G: 9 — see the tables; some are
shared-reference rows). Rows with no direct field (marked **(derived)**): A4,
A8, A9, B3, B4, C1, C2, C6, D1, D12, E7, E8, F2, F7.

## Gotchas for M2/M5

Confirmed by evidence unless marked (source).

1. **#2803** is printed for every `cerastes::` attribute; the attribute is still
   recorded (`is_invalid_namespace: TRUE`).
2. **Class-template attributes are NOT dropped** in this configuration
   (`attr_classtmpl.cpp`); the `attribute.c:10906` drop is not observed.
3. **Attributes after the parameter list sit on the function type**
   (`tk_routine` `source_corresp.attributes`), and **redeclarations lose them**
   (check 5). Contracts on a declaration plus a definition cannot both be read
   from the IL.
4. **Statement attributes are not printed by `il_display`**; use the walker.
5. **Dependent operations have type `tptk_unknown`** (`<unnamed>`), not `T`, not
   `bool`; only the `eok_bool_cast` wrapper around conditions is `bool`. `type_kind`
   is `tk_template_param` for most operations but `tk_unknown` for `eok_question`,
   `eok_call` and a cast inside a `tpck_expression`, and `tk_integer` for
   `eok_land`/`eok_lor` and constraint conjunctions.
6. **`tpck_expression` wrappers.** `(T)100`, `!(a < b)`, default arguments such as
   `(T)1`, and initializer expressions like `&r` are `enk_constant`s of
   `ck_template_param`; the real node is inside `constant.expr` (for a
   `dik_constant` initializer, `T *q = &r;` is an `eok_cast tk_unknown` over an
   `enk_constant` whose `tpck_expression` holds `eok_address_of`). A checker
   that walks `operands` only misses them.
7. **No `eok_parens`.** Parentheses are `is_parenthesized` on the node.
8. **Template parameter positions are 1-based** (`T#(1,1)`, `U#(1,2)`).
9. **`std::integral` lives in `std::__1`** (libc++ inline namespace).
10. **A type-constraint's `concept_id.args` is NULL** (implicit argument);
    requires-clause and trailing forms list `T` explicitly.
11. **Abbreviated `auto` parameters** are `template-parameter` with
    `is_abbreviated: TRUE` and no name.
12. **Prototype routines also carry `assoc_template` and `template_arg_list`** (the
    template's own parameters); distinguish instances by the absence of
    `is_prototype_instantiation`.
13. **No instance list per template.** Scan all routines.
14. **Instances exist only for used templates** (`tim_used`); a dependent call
    in a prototype body stays an unresolved `eok_call` with an `enk_constant`
    callee (no instance is created from it).
15. **A function body is a separate memory region**, reached by
    `function_def_number` -> scope (`sck_function`) -> `assoc_block`; il_display
    prints `Intermediate language for memory region N (function "name")`.
16. **Redeclared templates have one `a_template` per declaration**; the canonical
    entry has `definition_template`, all share one prototype routine
    (`generic_body.cpp` `declared_first`).
17. **`position.seq` is a sequence number**, not a line, when the file includes
    headers.
18. **Attribute tokens are re-spelled by EDG** (`attr_tokens.cpp`): `0x1F` ->
    `0x1f`, `9u` -> `9U`, `1.50` -> `(1.5)`, `1E3` -> `(1000.0)`, `07` -> `0x7`.
    M2 must not rely on the exact spelling of tokens (compare parsed values or
    use registered attributes whose arguments are parsed as expressions).
19. **#2803 hides today's silent drops.** The prefix attribute on a class or
    enum definition that declares nothing is dropped without a diagnostic other
    than #2803; when M2 registers the namespace the drop is fully silent (5b-class).
20. **Template arguments can be `tk_typeref`s.** `count_to<uint16_t>`'s entry is a
    `tk_typeref` to `unsigned short`; M5 must skip typerefs before classifying an
    argument (F5). `bool` is `int_kind: char` + `bool_type: TRUE`, so excluding it
    needs `bool_type` (B8).

## M3: scalar code (probes P1–P12)

Evidence for the facts the scalar encoder (decisions 0010 and 0012, `internal/encoding.md`) relies on. Probes are in
`tests/il/m3/` (header `frontend-ok`, `frontend-warning <tag>` or `golden`; each runs in `check.sh`). The listings below
come from `scripts/cerastes-fe.sh --c++20|--c23 --il_display <probe>` read back through a scratch printer that shows,
per expression node, its kind, `type`, `operation.type_kind`, flags, constant value, `a_constant.expr`/`orig_type`,
and the position (`position`) and start (`expr_range.start`) as `L<line>:<col>`; `compiler_generated` is the
`an_expr_node` flag (il_def.h:15092), `ol=` is `orig_lvalue_type` (:14915), `lvalue` is `is_lvalue` (:14940). Trimmed
excerpts are marked. Verdicts are PASS (the output shows it) or UNCLEAR; nothing is rounded up.

### P1: promotions and the usual arithmetic conversions: **PASS**

`tests/il/m3/p1_conversions.cpp` (C++20) and `p1_conversions.c` (C23).

- Every converted operand is its own `eok_cast`, `compiler_generated`, whose `type` is the converted type. The
  operation's `type` is the converted (promoted/common) type and `operation.type_kind` is `tk_integer`
  (il_def.h:15126-15132: the kind of type the operation acts on).
- Promotion of narrow operands (`p1_conversions.cpp:3`, `signed char + signed char`):
  ```
  eok_cast tk=tk_integer type=signed char compiler_generated @L3:56
    eok_add tk=tk_integer type=int @L3:58 start@L3:56
      eok_cast tk=tk_integer type=int compiler_generated @L3:56
        variable type=signed char a (param) ol=signed char @L3:56
      eok_cast tk=tk_integer type=int compiler_generated @L3:60
        variable type=signed char b (param) ol=signed char @L3:60
  ```
  The outermost cast is the implicit conversion of `return` back to the return type.
- `unsigned short * unsigned short` (`:5`) is `eok_multiply type=int` over two int-casts: computed in `int`.
- Mixed signedness: `int + unsigned` (`:7`) casts only the `int` operand (`eok_cast type=unsigned int`); `long + unsigned`
  (`:9`) casts the `unsigned` to `long`; `long long + unsigned long` (`:11`) is computed in `unsigned long long`
  (`eok_add type=unsigned long long`, both operands cast to `unsigned long long`, result cast back to `long long`).
- Comparison `int < unsigned` (`:12`): `eok_lt type=bool` over `eok_cast type=unsigned int` of the `int` operand. Same in C
  with `type=int` for the result (`p1_conversions.c:8`).
- Plain `char` operands are promoted like `signed char` (`ch1`, `:13`).

C23 gives the same shapes (`p1_conversions.c:4-8`). Counter-evidence for "every conversion is explicit": none found
for ordinary binary, unary, comparison and call-argument contexts; see P2 and P4 for the two places where the IL does
**not** convert (compound assignment, shift counts).

### P2: compound assignment and `++`/`--`: **PASS (with a finding that contradicts 0012 item 7)**

`tests/il/m3/p2_compound.cpp`, `p1_conversions.c:9`.

- `x op= e` is ONE node (`eok_add_assign`, `eok_subtract_assign`, `eok_multiply_assign`, `eok_divide_assign`,
  `eok_remainder_assign`, `eok_shiftl_assign`, `eok_shiftr_assign`, `eok_or_assign`), `type` = the left operand's type,
  `operation.type_kind` = `tk_integer`. The left operand is the bare lvalue. The right operand is **not converted to a
  computation type**:
  ```
  eok_add_assign tk=tk_integer type=signed char is_lvalue result_is_not_used @L3:38
    variable type=signed char c (param) is_lvalue @L3:36
    constant type=int int:1 [expr=NONE] @L3:41                 (c += 1)
  eok_add_assign tk=tk_integer type=int is_lvalue result_is_not_used @L6:50
    variable type=int i (param) is_lvalue @L6:48
    variable type=unsigned int u (param) ol=unsigned int @L6:53    (i += u: raw unsigned)
  eok_add_assign tk=tk_integer type=short is_lvalue result_is_not_used @L5:33
    variable type=short s (param) is_lvalue @L5:31
    variable type=long l (param) ol=long @L5:36                 (s += l: raw long)
  eok_multiply_assign tk=tk_integer type=short is_lvalue result_is_not_used @L5:41
    variable type=short s (param) is_lvalue @L5:39
    eok_cast tk=tk_integer type=int compiler_generated @L5:44   (s *= s: rhs promoted by its own cast)
      variable type=short s (param) ol=short @L5:44
  ```
  The right operand carries only the integer promotion the operand would get anywhere (a narrow rhs is cast to `int`);
  it is never converted to the common type with the left operand, and the left operand is never converted. So the
  computation type `C` of `x op= e` is **not in the IL**: it has to be derived (usual arithmetic conversions of the
  promoted left type and the right operand's type, for shifts the promoted left type alone).
- `++`/`--`: `eok_pre_incr`, `eok_post_incr`, `eok_pre_decr`, `eok_post_decr`, `type` = operand type, no conversion node
  (`p2_compound.cpp:7-8`, `signed char`, `unsigned char`, `short`, `int`). Pre forms are lvalues in C++
  (`is_lvalue`) and not in C (`p1_conversions.c:9`: `eok_pre_incr ... type=signed char result_is_not_used`); post
  forms yield a prvalue of the operand type (`eok_post_incr type=signed char`), which a use converts like any other
  prvalue (`int a = c++;`, `p2_compound.cpp:11`: `eok_cast type=int compiler_generated` over `eok_post_incr`).
- `++b`/`b++` on `bool` is an error in C++ (EDG: "incrementing a bool value is not allowed"); in C it is accepted:
  `p5_bool.c:14`, `eok_pre_incr tk=tk_integer type=bool result_is_not_used`.
- `bool b; b += i; b |= true;` (C++, `p2_compound.cpp:10`): `eok_add_assign type=bool` with the raw `int` right operand,
  and `eok_or_assign type=bool` with the right operand a constant `int:1` whose `expr` is an `eok_cast` from the `bool`
  constant.
- Result unused: every statement-level assignment has `result_is_not_used` (il_def.h expr-node flag).

Fields: `an_expr_node.type` (il_def.h:14903 region), `operation.kind`/`type_kind` (:15126-15132), `result_is_not_used`,
`is_lvalue` (:14940).

### P3: folded constants, diagnostics, pragmas: **PASS (cases below); one UNCLEAR**

`tests/il/m3/p3_folded.cpp`, `p3b_default_arg_constants.cpp`; the tool outputs are reproduced.

**What the IL keeps.** `a_constant.expr` (il_def.h:4777-4790; `orig_type` :4774) is kept for every constant in a function
body that is the result of folding or of a conversion:

| Source (`p3_folded.cpp`) | IL in the body |
|---|---|
| `INT_MAX + 1` (`:18`) | **not folded**: `eok_add tk=tk_integer type=int @L18:33` over `constant 2147483647`, `constant 1` |
| `65536 * 65536` (`:31`), `-(-2147483647 - 1 - 1)`, `9223372036854775807LL + 1` | not folded either: the operation stays, operands are constants |
| `1 / 0`, `1 % 0` (`:20`, `:21`) | not folded: `eok_divide` / `eok_remainder` over `constant 1`, `constant 0 [is_simple_zero]` |
| `1 << -1` (`:26`) | not folded: `eok_shiftl` over `constant 1` and `constant -1 [expr=eok_negate]` |
| `1 << 40` (`:19`) | **folded** to `constant type=int int:0 [expr=eok_shiftl]`; `expr` = `eok_shiftl` over `1` and `40` |
| `signed char c = 300;` (`:22`) | `dyninit dik_constant`, `const int:44 orig_type=int expr=present`; `expr` = `eok_cast type=signed char compiler_generated` over `constant int:300` |
| `unsigned u = -1;` (`:23`) | `const int:4294967295U orig_type=int`, `expr` = `eok_cast type=unsigned int` over `constant -1 [expr=eok_negate]` |
| `(short)70000` (`:24`) | `constant type=int int:4464 [expr=eok_cast,orig_type=short]`; its `expr` chain ends at `eok_cast type=short` (**not** compiler_generated: the explicit cast) over `constant int:70000` |
| `x + (2147483647 + 1)` (`:25`) | `eok_add` over `variable x` and `eok_add is_parenthesized` over the two constants: not folded |
| `0u - 1` (`:28`) | folded to `int:4294967295U [expr=eok_subtract]` then cast to `int` with `orig_type=unsigned int`; expr chain intact |
| `return 300;` in `signed char` function (`:37`), `takes_sc(300)` (`:39`) | `constant type=signed char int:44 [expr=eok_cast,orig_type=int]`, `expr` = `eok_cast ... signed char compiler_generated` over `constant 300` |

Excerpt (`p3_folded.cpp:24`, `(short)70000`):
```
constant type=int int:4464 [expr=eok_cast,orig_type=short] @L24:29
  -- constant.expr:
    eok_cast tk=tk_integer type=int compiler_generated @L24:29
      constant type=short int:4464 [expr=eok_cast,orig_type=int] @L24:29
        -- constant.expr:
          eok_cast tk=tk_integer type=short @L24:29
            constant type=int int:70000 [expr=NONE] @L24:36
```
So: the original expression is available for every folded or converted constant that occurs under a function-body
node. Literals have `expr=NONE` (a literal needs none). **UNCLEAR:** whether a constant with `orig_type` and no `expr`
(the "implicitly-converted" case of the il_def.h comment) can occur in a function body. It occurs only at file scope in
the corpus (40 constants such as `0UL` in system headers, all under file-scope nodes, 0 under a function-region node,
over the 115 goldens' il_display and the M3 probes; none of the 40 prints `local_expr_ref`, which il_display prints for
a constant when set, il_display.c:1353), and il_def.h:4774-4790 says `expr` is NULL for "some implicitly-converted
constants" and when "memory region constraints do not permit direct pointing" (then `local_expr_ref` is TRUE).
Default-argument constants copied into a call do keep `expr` (`p3b_default_arg_constants.cpp:4`, `constant ...
int:7L [expr=eok_cast,orig_type=int] @L4:25`). A constant that has `orig_type` and no `expr` should be rejected rather
than trusted; none of the probes reaches that rule.

**Diagnostics** (`scripts/cerastes-fe.sh --c++20 --display_error_number --brief_diagnostics --no_colors`, `p3_folded.cpp`):

| Case | Number, tag | Fires at |
|---|---|---|
| `INT_MAX + 1`, `65536 * 65536`, `9223372036854775807LL + 1`, `-2147483647 - 1 - 1`, `x + (2147483647 + 1)` | #61 `integer_overflow` | the operator |
| `1 << 40` | #63 `shift_count_too_large` | the count |
| `1 << -1` | #62 `negative_shift_count` | the count |
| `1 / 0` | #39 `divide_by_zero` | the divisor |
| `1 % 0` | #179 `mod_by_zero` | the divisor |
| `signed char c = 300`, `return 300` (signed char), `takes_sc(300)` | #69 `integer_truncated` | the constant |
| `unsigned u = -1`, `0u - 1` | #68 `integer_sign_change` | the constant |
| `(short)70000` | **none** | (explicit cast: silent) |

`p3c_unsigned_diagnostics.cpp`: `unsigned char c = 300;` and `unsigned short f() { return 70000; }` also give #69, and `unsigned u = -1;` gives #68. So
#68 and #69 fire for conversions **to unsigned**, which 0010 item 5 makes legal (modular). Making them errors would
reject valid programs, and the cases they cover are all visible to the encoder through `expr` (table above), so they
need not be enabled. #39, #61, #62, #63, #179 cover cases that are not folded anyway (the operation stays in the IL);
they are a backstop. One false positive: `(__int128)9223372036854775807LL * 4` gives #61 although it fits in 128 bits
(`p9_wide_constants.cpp:11`, the single warning that probe expects), and is left unfolded; with `--diag_error=61` valid 128-bit code would be rejected.

`--diag_error=<n,...>` turns these into errors (`... error #61-D: integer operation result is out of range`, "2 errors
detected", exit status 2; the same file exits 0 without it).

**Pragmas** (`p3d_diag_pragmas.cpp` shows the default-option diagnostics; the `--diag_error=61` runs are from the same text on the command line): `#pragma diag_suppress 61` and
`#pragma diag_remark 61` silence the diagnostic; `#pragma diag_default 61` restores it; **`#pragma diag_warning 61` makes it
a warning even under `--diag_error=61`** (the pragma overrides the command line: line 10 stays `warning #61-D` while
lines 2 and 6 become `error #61-D`). `#pragma clang diagnostic ignored "-Winteger-overflow"` is **not** honoured
(`warning #161-D: unrecognized #pragma`, line 11; the overflow warning at line 13 still fires); `#pragma GCC diagnostic`
is `warning #1675-D: unrecognized GCC pragma`. The diagnostic pragmas leave no trace in the IL (only `checking_pragma`
entries appear), so rejecting them needs the source scan, as 0010 item 12 says.

### P4: shapes of `<<`, `>>`, `%`, unary operators, `!`, comma, `?:`: **PASS**

`tests/il/m3/p4_shapes.cpp`.

- Shifts are NOT balanced: `eok_shiftl`/`eok_shiftr` with `type` = the promoted LEFT operand type; the right operand is raw
  (`a << u`: `variable type=unsigned int u` uncast, `:3`; `a >> l`: raw `long`; `uc << 3`: left cast to `int`, `3` raw):
  ```
  eok_shiftl tk=tk_integer type=int @L3:109 start@L3:106
    eok_cast tk=tk_integer type=int compiler_generated @L3:106
      variable type=unsigned char uc (param) ol=unsigned char @L3:106
    constant type=int int:3 [expr=NONE] @L3:112
  ```
  `unsigned long a << int n` (`:4`): `eok_shiftl type=unsigned long`, raw `int` count.
- `%` and `/` are balanced like `+`: `u % b` with `int b` gives `eok_remainder type=unsigned int` with `b` cast to
  `unsigned int` (`:5`).
- `-a` is `eok_negate`, `+a` is `eok_unary_plus`, `~a` is `eok_complement`; operands of narrow type are promoted by a cast
  (`-c` on `unsigned char`: `eok_negate type=int` over `eok_cast type=int`); `-u` on `unsigned` stays `unsigned int`
  (`(int)-u`: explicit `eok_cast tk=tk_integer type=int` (not compiler_generated) over `eok_negate type=unsigned int`).
  `-b` on `bool` promotes to `int` through a cast.
- `!a` (C++): `eok_not type=bool` over `eok_bool_cast type=bool compiler_generated` of the `int` operand; assigning it to an
  `int` adds `eok_cast type=int compiler_generated`.
- `&&`/`||` (C++): `eok_land`/`eok_lor type=bool`, each operand an `eok_bool_cast` when not already `bool`;
  `is_parenthesized` marks `(...)`; there is no parenthesis node.
- `?:`: `eok_question tk=tk_unknown type=<common type>` with operands (condition, then, else); `c ? a : u` with
  `int a`, `unsigned u` has `type=unsigned int` and `a` wrapped in `eok_cast type=unsigned int`
  (`:9`).
- Comma: `eok_comma tk=tk_unknown type=int` with two operands (`:10`); pre/post increments inside expressions keep their
  own nodes (`:11`: `eok_add` over `eok_post_incr` and `eok_pre_incr`).
- `type_kind` is `tk_integer` for every integer operation above and `tk_unknown` for `eok_question`, `eok_comma`,
  `eok_call` (il_def.h:15126-15132).

### P5: `bool`, C23 `bool`/`_Bool`, results of comparisons and logic: **PASS**

`tests/il/m3/p5_bool.c` (C23) and `p5_bool.cpp` (C++20).

- C23: comparison, `&&`, `||` and `!` have `type=int` (`p5_bool.c:4`: `eok_lt tk=tk_integer type=int`); their operands
  are NOT wrapped (`eok_land type=int` over the bare `variable a` and `variable b`, `:5`), and `if`/`while` conditions
  are the bare `int` expressions (`stmk_if` with `variable type=int a`, `:13`).
  C++20 for contrast: `eok_lt type=bool`, wrapped in `eok_cast type=int compiler_generated` when used as `int`
  (`p5_bool.cpp:4`); `&&` operands are `eok_bool_cast compiler_generated` (`:5`); the `if` condition is
  `eok_bool_cast type=bool compiler_generated` over `variable a` (`:12`).
- Conversion to `bool`: `eok_bool_cast tk=tk_integer type=bool compiler_generated` in both languages (`bool as_bool(int a)`,
  `p5_bool.c:7`, `p5_bool.cpp:7`); in C also `bool r = a < b;` is `eok_bool_cast` over `eok_lt type=int` (`:8`).
- From `bool`: `eok_cast tk=tk_integer type=int compiler_generated` (`b + 1`, `:10`).
- `true`/`false`: `constant type=bool int:1 [expr=NONE,non_arithmetic]` (C++, `p5_bool.cpp:10`); in C23 the printed value is
  `int:1U` / `int:0U` (`p5_bool.c:11-12`), the same constant with an unsigned-style suffix.
- `bool` itself is `tk_integer` with `int_kind: char` and `bool_type: TRUE` (P12).

### P6: parameter references: contract, body, statement annotations: **PASS**

`tests/il/m3/p6_param_refs.cpp` (golden; every fact of it was checked by `scripts/walker_oracle.py`, 12 facts).

- **Contracts** (`requires`/`ensures`, file-scope region): `enk_param_ref` with `param_ref.param_num` (1-based) and
  `param_ref.levels_up`. From the raw il_display of the probe:
  ```
  kind:                    enk_param_ref
  param_ref.param_num:     1
  param_ref.levels_up:     0         (requires: parameter 1, `a`)
  ```
  and `param_num: 2, levels_up: 0` for `b`. In `ensures` the parameters are `{k,1}` and the result binder is `{1,0}`
  (walker listing `ensures(r, r >= a)`: `binder r: int @ L6:76`, `param 1 a: int @ L6:81`), as annotation_il.h documents.
- **Body**: `enk_variable` whose `variable` is the parameter variable (`variable ... a (param)`, `is_parameter: TRUE`,
  listed in the function scope's `parameters`). `int s = a;` is `variable type=int a (param)` (`p6_param_refs.cpp:8`);
  the assignment `a = a + 1;` has `variable type=int a (param) is_lvalue`.
- **Statement annotations** (`check`, `invariant`): `enk_variable` of the SAME variable records, in the function
  region, with `position.orig_*` = the macro-argument position (raw il_display: `variable: func-scope variable@...: s`,
  `start.orig_column: 22`). The walker's tree: `var a: int decl=L6:11 @ L11:18` (`stmt stmk_empty @ L11:3 attr
  [[cerastes::check]]`), where `a` is the parameter variable and denotes its current value.
- **Mapping** parameter k <-> variable: the variable record has `assoc_param_type` (il_def.h:11232) pointing to the k-th
  `param-type` entry of the routine type (`param_num` k, il_def.h:6500s), so a contract's `{k,0}` maps to the variable
  whose `assoc_param_type` is the k-th param-type, and equally to the k-th entry of the function scope's `parameters`
  list. Evidence: `variable@...: a` has `assoc_param_type: param-type@778f9bdbe0`, whose `param_num: 1`; `b` has
  `param-type@778f9bdd10`, `param_num: 2`.
- il_display does not link the statement-annotation nodes to anything (it does not print the attribute entries of
  statements); they are found as expression nodes of the function region that no body reaches. The walker, whose
  output the oracle checks, is the reliable way to read them.

### P7: calls: **PASS**

`tests/il/m3/p7_calls.cpp`.

- `eok_call tk=tk_unknown type=<result type>`; operands: first an `enk_routine` (`routine type=int (*)(int, int) add
  ol=int (int, int)`), then the arguments in order. The overload is already selected: `overloaded(x)` and
  `overloaded(l)` name `int (*)(int)` and `int (*)(long)` respectively (`:20`).
- Arguments are converted to the parameter type by `eok_cast ... compiler_generated`: `add(s, uc)` casts both to `int`
  (`:14`); `narrow(x)` with `signed char` parameter casts `int x` to `signed char` (`:16`); `wide(x)` casts to `long`.
  The call result is converted by a cast around the call (`r = narrow(x)` is `eok_cast type=int` over `eok_call
  type=signed char`).
- **Default arguments** are inserted as copies, no `default_arg_expr` node: `with_default(x)` has operands `variable x`,
  `constant type=int int:5 ... @L7:33`, `constant type=long int:7L ... @L7:45`; the constants' positions are in the
  callee's declaration (L7), not at the call (L18). The copy's top node carries `generated_default_arg: TRUE`
  (il_def.h:14980): raw il_display shows `generated_default_arg: TRUE` on `expr-node@7ac16a3200` (kind `enk_constant`,
  position L7:33) and on the inserted `eok_call` of `int c = add(1, 2)` (position L25:48).
  A non-constant default (`c = add(1, 2)`) is the whole call node copied; an `enk_constant` default `2 + 3` is folded with
  `expr=eok_add` (`:25`).
- A function declared without a body is an ordinary routine with no function scope (printer: `== int f(n: int) @L4:5 nobody`
  for `p11_statements.cpp:4`): the call node is identical to a call of a defined function
  (`routine type=int (*)(int) nobody ol=int (int) @L15:7`, `p7_calls.cpp:15`).
- A call used as a statement has `result_is_not_used` and type `void` for `v()` (`:22`).

### P8: positions: **PASS**

`tests/il/m3/p8_positions.cpp` (the listing below is its il_display, positions as `L<line>:<col>`); more evidence in the P1-P7 excerpts.

```
r = (a + b) * 2;     eok_add (parenthesized)   position L3:14 start L3:12 end L3:16
                     eok_multiply              position L3:19 start L3:11 end L3:21
r = -r;              eok_negate                position L4:7  start L4:7  end L4:8
                     eok_assign                position L4:5  start L4:3  end L4:8
r = g(a, b);         eok_call                  position L5:8  start L5:7  end L5:13
return r > 0 ? r : -r;  eok_question           position L6:16 start L6:10 end L6:23
```
- A binary or assignment node's `position` is the OPERATOR token; `start` (`expr_range.start`) is the first operand's start
  and `end` the last token (`EXTRA_SOURCE_POSITIONS_IN_IL`). A call's `position` is the `(`; unary: the operator
  (at `start`); `?:`: the `?`; a compiler-generated cast: the operand's start; a statement: its first token
  (`stmk_return @L6:3`, `end_position` L6:24).
- `start` does not include a parenthesis around the node itself (`(a + b)`: start L3:12, the `(` is col 11) but the enclosing
  node's start does (`eok_multiply` start L3:11).
- `position.seq` is a sequence number mapped to a line by the `seq-number-lookup-entry` records (same as M1).
- **Macro arguments** (`CERASTES_*`): the nodes of an annotation argument have `position` = the macro invocation (L9:3 for a
  statement macro) and `position.orig_seq/orig_column` = the argument token in the user's text (raw il_display, P6:
  `start.orig_column: 22`, i.e. `L12:22`); the walker/back end uses the original when it is not in a system header. Nodes
  of ordinary code outside macros have no `orig_*`.
- A statement or expression with no source (`stmk_return` generated at a function's end, the `0` of `main`) has
  `position.seq` 0 and shows as `@-`.

### P9: reading full-width constants: **PASS**

`tests/il/m3/p9_wide_constants.cpp`.

- `an_integer_value` (targ_def.h:1486-1679) is a host integer here (`INTEGER_VALUE_REPR_IS_A_HOST_INTEGER` TRUE); the
  exact decimal text of any integer constant is `decimal_str_for_integer_constant(a_constant*)` (const_ints.h:254, defined
  :2230: value, signedness (`int_constant_is_signed`) and size taken from the constant's own type). The walker already
  uses it (`render_constant`, `src/backend/render.cpp`). il_display prints the same value (with a type suffix) in
  `integer_value`.
- Values read back (il_display `integer_value`, `p9_wide_constants.cpp`):
  `unsigned long long` max `18446744073709551615ULL`; `long long` min `-9223372036854775808LL` (folded from
  `-9223372036854775807LL - 1`, expr kept); `unsigned long` `9223372036854775808UL`; `__int128` `(1 << 100)`:
  `1267650600228229401496703205376`; `unsigned __int128` max: `340282366920938463463374607431768211455U`;
  `__int128` min: `-170141183460469231731687303715884105728`; `unsigned` max `4294967295U`.
- **il_display prints a type suffix** on integer values (`2UL`, `4294967295U`, `5LL`, `1U`); strip it when parsing il_display
  text. The API string has no suffix.
- A 128-bit literal in source does not compile (`error #23: integer constant is too large` for a 39-digit literal), so 128-bit constants arise only from folding casts (`(__int128)1 << 100`) or runtime values.
- Caveat (P3): `(__int128)9223372036854775807LL * 4` gets warning #61 although it fits, and stays unfolded.

### P10: const globals, enumerators, `constexpr`: **PASS**

`tests/il/m3/p10_named_constants.cpp`.

- A `const int` global with a constant initializer, used in a body, is a folded constant with its value AND an `expr`:
  `constant type=int int:40 [expr=enk_variable]` whose `expr` is `variable type=int kConst` (a reference to the global;
  `:12`). `constexpr int kCx = kConst + 2` gives `int:42 [expr=enk_variable]` (`:13`). A non-const global is a plain
  `variable type=int g_nonconst` with no constant (`:18`).
- An enumerator: `constant type=int int:6 [expr=eok_cast,orig_type=E]` whose `expr` is `eok_cast ... compiler_generated`
  over `constant type=E int:6 [expr=eok_add]`, whose `expr` is the initializer expression at its own position
  (`eok_add @L6:23`, in the enum declaration). `signed char sc = C;` (value 300) is `const int:44 orig_type=E`
  with a cast to `signed char` in `expr` and warning #69.
- `constexpr` calls: `sq(3)` is `constant type=int int:9 [expr=eok_call]` with `expr` = `eok_call` of `routine sq` with
  the argument constants; `big(0) + x` is `constant int:2147483647 [expr=eok_call]`: the folded value is kept AND the call
  is visible, so the encoder can see that a `constexpr` function ran at compile time. A non-constant argument (`sq(x)`)
  is an ordinary `eok_call`.

### P11: statements: **PASS**

`tests/il/m3/p11_statements.cpp`.

- `int s = 0;` is TWO statements: `stmk_decl` (with `entities` = the variable; the variable also has `dynamic`
  = the dynamic-init) followed by `stmk_init` (`dynamic_init` = the same dynamic-init, il_def.h:15833). The `stmk_init`
  is the one that executes; reading both double-counts the initializer. An initializer that is a constant is
  `dik_constant` (`const int:0 expr=NONE`); an expression is `dik_expression`. `int uninit;` has `init_kind=initk_none`
  (il_def.h:11016) and no `stmk_init`.
- `static_assert(...)` is a `stmk_decl` whose `entities` is a **`static-assertion`** record (`condition` = a constant
  `true`, `string_literal`), not a variable (`p11_statements.cpp`, function `sa`; il_display `func-scope static-assertion@...`, `condition: ... constant: true`).
- `for (int i = 0; i < n; i++) s += i;`: `stmk_for` with `initialization` (a `compiler_generated` block holding
  the `stmk_decl`/`stmk_init`), `expr` = the condition, `increment`, `statement` = the body (a `compiler_generated` block when the
  source has no braces), `for_init_scope`. A missing condition (`for(;;)`) has no `expr`. `j++, k++` in the increment
  is `eok_comma`.
- `while`: `stmk_while` with `expr` and `loop_statement`. `if`/`else`: `stmk_if` with `expr`, `then_statement`,
  `else_statement` (a source `else if` is a `compiler_generated` block holding a nested `stmk_if`).
- `break` and `continue` are `stmk_goto compiler_generated` (and a `stmk_label` after the loop); the encoder must reject
  every `stmk_goto`, including generated ones.
- `if (int x = f(n))`: the condition is an expression node of kind `enk_condition` (il_def.h:13593) at no position.
- `main` without `return`: a trailing `stmk_return compiler_generated` at `@-` with a constant `0`. A non-`void` function that
  falls off its end (`falls_off`): a trailing `stmk_return compiler_generated` with **no** expression (and EDG warns #940
  `implicit_return_from_non_void_function`); a `void` function also ends in a generated `stmk_return` without expression.
  So "closing brace reached in a non-`void` function" is: a generated `stmk_return` without `expr`.
- `return a;` in a `signed char` function: `stmk_return` with `eok_cast type=signed char compiler_generated` over `variable a`.
- A nested block `{ int inner = s; ... }` is a `stmk_block` with its own scope.

### P12: integer types: size and signedness: **PASS**

`tests/il/m3/p12_type_facts.cpp` (C++20; `_BitInt` accepted in C++).

Every integer type is `kind: tk_integer` with `int_kind` (`an_integer_kind`, il_def.h:5659-5679, field :10052), `size`
in bytes (a_type.size, il_def.h:9793), and flags (`bool_type` :10108, `char16_t_type`/`char32_t_type` :10096):

| Type | `int_kind` | `size` | notes |
|---|---|---|---|
| `bool` | `char` | 1 | `bool_type: TRUE` (C++ and C23 alike) |
| `char` | `char` | 1 | signedness: il_header `plain_chars_are_signed: TRUE` (il_def.h:18828), `int_kind_is_signed[ik_char]` (:5689) |
| `signed char` / `unsigned char` | `signed char` / `unsigned char` | 1 | |
| `short` / `unsigned short` | `short` / `unsigned short` | 2 | |
| `int` / `unsigned` | `int` / `unsigned int` | 4 | |
| `long`, `long long` and unsigned | `long`, `long long`, ... | 8 | LP64 |
| `wchar_t` | `int` | 4 | printed name `wchar_t`; same `int_kind` as `int` (signed here) |
| `char8_t` | `unsigned char` | 1 | |
| `char16_t` / `char32_t` | `unsigned short` / `unsigned int` | 2 / 4 | `char16_t_type`/`char32_t_type` flags |
| `__int128` / `unsigned __int128` | `__int128_t` / `__uint128_t` | 16 | `ik_int128`, `ik_unsigned_int128` |
| `_BitInt(24)` | `_BitInt(24)` | 3 | `ik_bit_precise`; width in the name, not in `size * 8` |
| `unsigned _BitInt(7)` | `unsigned _BitInt(7)` | 1 | `ik_unsigned_bit_precise` |

Output (scratch reader over `p12.il`): `f_bool [('tk_integer','char','bool','1')]`, `f_wc [('tk_integer','int','','4')]
wchar_t`, `f_i128 [('tk_integer','__int128_t','','16')]`, `f_bi [('tk_integer','_BitInt(24)','','3')]`. So: size from the type's
`size`; signedness from `int_kind_is_signed[int_kind]` at run time (plain `char` after EDG has set it from the target); `bool` by
`bool_type`; `_BitInt` by the two bit-precise kinds (reject); enumerations are `tk_integer` with `variant.integer.enum_type` set
(il_def.h:5583 `tk_enum = tk_integer`, a synonym, not a kind of their own; P10: the constant's type prints as `E`).

### Probe-by-probe answers to decisions 0010 and 0012

| Reference | Verdict | Evidence |
|---|---|---|
| 0010 item 2 (P12): sizes and signedness from EDG at run time; plain `char` signed on this target | **confirms** | P12 table (`size`, `int_kind`, `plain_chars_are_signed: TRUE`); the array `int_kind_is_signed` itself is not visible in il_display (UNCLEAR for the array, PASS for the header flag it comes from) |
| 0010 item 2 (P9): `__int128` supported only if exact constant reading exists | **confirms** it exists | `decimal_str_for_integer_constant` (const_ints.h:254); il_display prints full 128-bit values; caveat: spurious #61 on valid 128-bit multiplies (not folded) |
| 0010 item 6 (P1, P2): "the IL makes promotions and usual arithmetic conversions explicit" | **confirms P1; contradicts P2** | P1: every binary/unary/comparison/call-argument conversion is an `eok_cast compiler_generated`. P2/P4: compound assignment, `<<=`/`>>=` and shift counts are NOT converted: the right operand is raw (or only integer-promoted) and the computation type is not in the IL |
| 0010 item 10 (P5): C result `int`, C++ `bool` | **confirms**, with a refinement | C23: comparison/logical/`!` nodes have `type=int`, operands and conditions are not wrapped; C++: `type=bool`, operands/conditions wrapped in `eok_bool_cast` |
| 0010 item 12 / P3: "constant keeps its original expression, or only `orig_type`" | **confirms** for function bodies; one **UNCLEAR** | all converted/folded constants in bodies keep `expr` (table above); the no-`expr` form was found only at file scope; overflowing operations (`INT_MAX + 1`, `65536 * 65536`, `1/0`, `1 << -1`) are not folded at all |
| 0010 item 12: `--diag_error` for #61/#39/#63..., "tags fixed by P3" | **confirms** the mechanism, amends the list | #39, #61, #62, #63, #179 work (`--diag_error=...` -> errors); #68/#69 also fire for conversions to unsigned (legal, modular), so they must not be enabled; a user `#pragma diag_warning` overrides `--diag_error` (so reject diag pragmas, as written); `#pragma clang/GCC diagnostic` are not honoured (#161/#1675) |
| 0012 item 2 (P6): contracts read formals, `check`/`invariant` read current locals | **confirms** | contracts use `enk_param_ref` `{k,0}` (`{k,1}` and binder `{1,0}` in `ensures`); body and statement annotations use `enk_variable` of the parameter variables; the mapping is `assoc_param_type` |
| 0012 item 7 (P2): "computation type `C` the IL gives" | **contradicts** | P2: the IL does not give `C` for compound assignment; the encoder must derive it (usual arithmetic conversions of the promoted left type and the right operand), and for shifts use the promoted left type |
| 0012 item 12: "globals (unless P10 shows the IL gives a constant)" | **confirms** the exception | P10: a `const`/`constexpr` integer global is a folded constant whose `expr` is an `enk_variable` of the global; a non-const global is a bare `enk_variable` |
| 0012 item 6 calls (not tagged P7 in the text) | **confirms** | P7: operand 1 `enk_routine` (overload selected), converted arguments, default-argument copies with `generated_default_arg`, results converted by a cast around the call; a bodyless function is an ordinary routine |
| 0012 item 3 statements (not tagged) | **confirms with additions** | P11: declaration + `stmk_init` pair; `static_assert` is a `stmk_decl` over a `static-assertion` entity; `break`/`continue` are generated `stmk_goto`s; `if (int x = ...)` has an `enk_condition` |
| 0012 item 9: falling off the end, `main` returns 0 | **confirms** | P11: generated trailing `stmk_return`, with constant 0 only in `main` |
| 0012 item 12: vector arithmetic uses the ordinary operator kinds, operand types are checked too | **UNCLEAR** | not probed here (EDG bug 0007 vector types); `operation.type_kind` is `tk_vector` per il_def.h:15126-15132's comment, not observed |
| encoding.md section 2: shifts typed as "promoted left operand type" | **confirms** | P4 |
| encoding.md section 3: positions of obligations | **confirms the rule needed** | P8: error positions should use the operator token (`position`); for default arguments and constants from the callee's declaration use the call's position, since their own positions lie in the declaration |

### R4a: constant-evaluated uses of `constexpr` predicate and pure functions

`tests/il/m3/r4a_constant_contexts.cpp` (C++20; `q` is a `constexpr CERASTES_PREDICATE` returning `bool`, `p` a
`constexpr CERASTES_PURE` returning `int`). Every context below calls one of them in a constant expression. The traces
come from il_display read through a scratch scan (`r4a_scan.py` in the session scratchpad): for every `enk_routine`
expression node naming `p` or `q`, its chain of owners by reverse pointer search (the `<key-` links below are the IL
fields that lead from the owner to the node). The same text through `scripts/cerastes-fe.sh --c++20 --brief_diagnostics
--display_error_number --no_colors` gives what the back end reports today. Line numbers are those of the probe.

| Context | Line | Trace in the IL (owner `<field-` chain from the `enk_routine` of `p`/`q`) | Where | Reported today | Found by scanning every expression node |
|---|---|---|---|---|---|
| C1 array bound (namespace scope) | 23 | `type(tk_array) <bound_constant- constant <expr- eok_call <operands- enk_routine` (the bound constant also appears as an `eok_cast`'s `constant`) | file-scope region | no | yes |
| C1b array bound (local) | 43 | `local-expr-node-ref <expr` (record `constant-expr referrer: constant`, listed in the function scope's `expr_node_refs`) | function region | no | yes |
| C2 template argument (class template) | 25 | **none**: the instance `S<4>` carries `template_arg_list` with `constant: 4` (`ck_integer`, no `expr`, no routine pointer anywhere) | - | no | **no trace** |
| C2b template argument (local) | 44 | **none** (`S<2>`, same) | - | no | **no trace** |
| C19 explicit function template argument `ft<p(1)>()` | 52 | **none**: the instance routine's `template_arg_list.constant: 2` has no `expr`; the call node is `eok_call` of `routine ft` only | - | no | **no trace** |
| C3 `static_assert` at namespace scope | 26 | **none**: no `static-assertion` record and no node naming `q` (only block-scope ones leave a record) | - | no | **no trace** |
| C3c `static_assert` in a class | 38 | **none** (same) | - | no | **no trace** |
| C3b `static_assert` in a body | 45 | `static-assertion <condition- constant <expr- eok_call <operands- enk_routine`; the `stmk_decl`'s `entities` points to the record | function region | no | yes |
| C4 `case` label | 46 | `switch-case-entry <case_value- constant <expr- eok_call <operands- enk_routine` | function region | yes (L46) | yes |
| C5 enumerator initializer (namespace) | 27 | named `constant(ck_integer)` (the enumerator) `<expr- eok_call`; the enumerator's constant is also reached through an `eok_cast` node | file-scope region | no | yes |
| C5b enumerator in a body | 47 | same shape; the enumerator's entries live in the file-scope region although the enum is local | file-scope region | no | yes |
| C6 `constexpr` variable (namespace) | 28 | `variable(g_cx) <constant- constant <expr- eok_call <operands- enk_routine` | file-scope region | no | yes |
| C6b `constexpr` variable (local) | 48 | `variable(local_cx) <dynamic- dynamic-init <constant- constant <expr- eok_call` | function region | yes (L48) | yes |
| C7 `if constexpr` | 49 | `stmt(stmk_constexpr_if) <expr- enk_constant <constant- constant <expr- eok_call` | function region | yes (L49) | yes |
| C8 `noexcept(...)` operator | 50 | `variable(n) <dynamic- dynamic-init <constant- constant <expr- eok_noexcept <operands- eok_call` | function region | yes (L50) | yes |
| C9 `alignas(p(7))` | 32 | `attribute-arg <constant- constant <expr- eok_call` (the `alignas` is an attribute entry on the variable; its argument is an `attribute-arg` of kind `constant`) | file-scope region | no | yes |
| C10 bit-field width | 31 | `field <bit_size_constant- constant <expr- eok_call` | file-scope region | no | yes |
| C11 default template argument | 29 | `template-parameter(N) <default_arg_constant- constant <expr- eok_call`; the instance `D<6>` has no trace | file-scope region | no | yes (on the template parameter) |
| C12 body of a `constexpr` function | 33 | `stmt(stmk_return) <expr- eok_add <operands- eok_call <operands- enk_routine` | function region | yes (L33) | yes |
| C13 body of a `consteval` function | 34 | `stmt(stmk_return) <expr- eok_call <operands- enk_routine` | function region | yes (L34) | yes |
| C12b/C13b constexpr/consteval call, constant-evaluated or not | 53-55 | the call site is an ordinary `eok_call` of `ce_body`/`cx_body` (folded to a constant with the call kept as `expr`); the use of `p` is only in the callee's body (C12/C13) | - | via the callee (L33/L34) | yes (callee body) |
| C14 `noexcept(q(1))` specifier | 36 | `exception-specification <noexcept_arg- constant <expr- eok_call` | file-scope region | no | yes |
| C15 `explicit(q(1))` | 37 | `attribute-arg <constant- constant <expr- eok_call` | file-scope region | no | yes |
| C16 `sizeof(p(1))` (unevaluated) | 51 | `variable(sz) <dynamic- dynamic-init <constant- ... eok_cast <expr- constant <expr- enk_sizeof <expr- eok_call` | function region | yes (L51) | yes |
| C17 `decltype(p(1))` (unevaluated) | 40 | `type(tk_typeref) <expr- eok_call <operands- enk_routine` | file-scope region | no | yes |

Output of the scan for the rows with a trace, quoted (the `CALL` abbreviates `expr-node(routine) <operands-
expr-node(operation call)`): `(23, 'p', 'file-scope', 'CALL <expr- constant(ck_integer) <constant- expr-node(constant)
<operands- expr-node(operation cast) <expr- constant(ck_integer) <bound_constant- type(tk_array:int [4])')`;
`(43, 'p', 'func-scope', 'CALL <expr- local-expr-node-ref')`; `(45, 'q', 'func-scope', 'CALL <expr- constant(ck_integer)
<condition- static-assertion')`; `(46, 'p', 'func-scope', 'CALL <expr- constant(ck_integer) <case_value-
switch-case-entry')`; `(32, 'p', 'file-scope', 'CALL <expr- constant(ck_integer) <constant- attribute-arg')`;
`(36, 'q', 'file-scope', 'CALL <expr- constant(ck_integer) <noexcept_arg- exception-specification')`; `(40, 'p', 'file-scope',
'CALL <expr- type(tk_typeref:int)')`. No row exists for lines 25, 26, 38, 44, 52.

**What walks reach.** The checks that exist (the "Reported today" column: L33, L34, L46, L48, L49, L50, L51) find the
uses that are statements or expressions of function bodies: they walk `stmt` trees and what the statements point to. They
do not see the contexts whose trace hangs off a type, a field, a template parameter, an exception specification, an
attribute argument, an enumerator or a block-scope `static_assert`/local array bound (C1, C1b, C3b, C5, C5b, C6, C9,
C10, C11, C14, C15, C17), although every one of those traces is an `enk_routine` expression node in a dumped region.

**What a scan of all expression nodes reaches.** `il_display` and the walker enumerate IL entries with the same calls
(`walk_file_scope_il`, `walk_routine_scope_il`; il_display.c:8707, 8743; `src/backend/walker.cpp` `collect`), and
those hand every entry, expression nodes included, to the callback. A pass over every `enk_routine` node whose
routine carries `cerastes::predicate` or `cerastes::pure` finds a trace for 20 of the 25 rows (all but C2, C2b, C19,
C3 and C3c). Legitimate uses must be excluded: nodes under the argument trees of registered `cerastes::` attributes
(including statement attributes, whose entries are reachable in memory through `statement.attributes` though
il_display does not print them) and the bodies of spec functions. The exclusion has to tell those apart from other
attribute arguments (C9 and C15 are `alignas` and `explicit`, ordinary attributes whose `attribute-arg` is a
constant with a call in `expr`).

**Contexts with no trace.** A template argument that is a call to a `constexpr` function (C2, C2b, C19) and `static_assert`
outside a function body (C3, C3c) leave nothing: the argument is the folded value (`ck_integer`, no `expr`), and EDG
keeps no record of a namespace- or class-scope `static_assert`. The default template argument keeps its expression on the
template parameter (C11), but a use of the template with it (`D<>`) leaves only `D<6>`.

**Verdict for plan R4a.** Step 2 (reject from the IL) covers 20 of the 25 contexts with a pass over all expression
nodes, including every one of the "not reported today" contexts that has a trace, but it cannot cover C2, C2b, C19, C3
and C3c: for those the IL holds no trace of the call. They need step 3 (an EDG mechanism that stops constant evaluation
of a routine carrying a registered attribute, which would make every one of them an ordinary error, traces or not) or
step 4 (documented gap).

**Status of the R4a contexts (2026-10-16).** The 12 contexts that have a trace and that the earlier checks did not report
(11 rows plus the default template argument, which keeps its expression on the template parameter) are reported by the
annotation checks' pass over every expression node (test-mutator's IL pass, which excludes the argument trees of registered
`cerastes::` attributes and the bodies of spec functions). The 5 contexts with no trace (C2, C2b, C19, C3, C3c) need the
EDG fork's commit "P": a registration flag, `RAF_NO_CONSTANT_EVALUATION`, that stops constant evaluation of a routine
carrying a registered attribute, which makes every one of them an ordinary error. Until that commit is pinned they stay a
documented gap; nothing in the IL can close them.

### P13: recognized attributes in verified code: **PASS (with the findings below)**

`tests/il/m3/p13_attributes.cpp` (C++20) and `p13_attributes.c` (C23), both goldens (every line checked by
`scripts/walker_oracle.py`: 31 and 29 facts, the rest unverifiable as stated); the entry lists below come from a scratch
reader of il_display (`attrs.py`: for every routine, function type, parameter, type, variable and field it follows
`attributes` and prints `kind | name | namespace | syntactic_location | family | position`).

**One list per entity.** Every entity has ONE singly linked list of `an_attribute` entries (field `next`), reached from
the entity: `source_corresp.attributes` for routines, variables, fields, types, namespaces, labels, templates and
constants (il_def.h:3615); `a_param_type.attributes` for parameters (:6545; the parameter's variable record also has an
`attributes` pointer); `a_statement.attributes` for statements (:16540). Each entry has `kind` (`an_attribute_kind`; a
recognized attribute has its own kind, anything else is `unrecognized`), `family`, `syntactic_location` (:3173), `name`
(:3224) and `namespace_name` (:3226), and `arguments`. A walker over those lists enumerates every attribute; the back end
reads `kind` for recognized ones and `name` for the rest. Where each lands (C++20 unless noted):

| Source | Entity and list | Entry (`kind | name | ns | location | family`) |
|---|---|---|
| `[[noreturn]] void nr_def(int)` (`:5`) | routine | `noreturn \| noreturn \| - \| prefix \| std` |
| `__attribute__((noreturn)) void g1(int);` / `void g2(int) __attribute__((noreturn));` / `[[gnu::noreturn]] void g3(int);` (`p13b_gnu_spellings.cpp`) | routine | `noreturn \| noreturn \| - \| prefix or postfix \| gnu`; `[[gnu::noreturn]]`: ns `gnu`, family `std` |
| `[[nodiscard]] int nd(int)` (`:6`) | routine | `nodiscard \| nodiscard \| - \| prefix \| std` |
| `[[deprecated("old")]] int dep(int)` (`:7`) | routine | `deprecated \| deprecated \| - \| prefix \| std`; argument `<aak_constant>` (the string) |
| `[[gnu::always_inline]] inline int ai(int)` (`:8`) | routine | `always_inline \| always_inline \| gnu \| prefix \| std` |
| `int ai2(int) __attribute__((always_inline));` (`:9`) | routine | **`unrecognized \| always_inline \| - \| postfix \| gnu`**: the same attribute is recognized in the prefix spelling above and not in this one |
| `__attribute__((const)) int cn(int)`, `__attribute__((pure)) int pu(int)` (`:11`, `:12`) | routine | `const \| const \| - \| prefix \| gnu`, `pure \| pure \| ...` |
| `[[maybe_unused]] int a` and `int b [[maybe_unused]]` as parameters (`:14`) | parameter (`a_param_type.attributes`) | `maybe_unused \| ... \| prefix` and `... \| declarator-id` |
| `struct [[nodiscard]] Tag` (`:15`) | type | `nodiscard \| nodiscard \| - \| tag name \| std` |
| `struct [[deprecated]] Old`, `enum [[deprecated]] En` (`:16`, `:17`) | type | entry on the type's list (printed after the flag line, see below) |
| `[[maybe_unused]] int unused = 0;` (`:19`) and `int cl __attribute__((cleanup(cleanup_fn))) = 1;` (`:20`) | local variable | **entry not dumped** by il_display (`attributes: file-scope attribute@...` with no record); the walker prints `attr [[maybe_unused]] on variable unused @ L19:5` and `attr [[cleanup(cleanup_fn)]] on variable cl @ L20:25` |
| `[[assume(x > 0)]];` (`:21`), `__attribute__((assume(x > 1)));` (`:22`) | null statement | `stmk_empty` with `attributes` set; entry not dumped; the walker prints `stmt stmk_empty @ L21:3 attr [[assume(<aak_expression>)]]` and the same for `L22` (C++: an expression argument; C23: raw tokens, `[[assume(x > 0)]]` in `p13_attributes.c:18`) |
| `if (c) [[likely]] { } else [[unlikely]] { }` (`:23`) | the `then`/`else` **block** statements | `stmk_block @ L23:25 attr [[likely]]`, `stmk_block @ L23:54 attr [[unlikely]]` (not the `if`) |
| `[[fallthrough]];` (`:24`) | null statement | `stmk_empty @ L24:31 attr [[fallthrough]]` |
| `[[]];` (`:25`) | null statement | `attributes` set (a pseudo entry of kind `empty attribute`), no entry printed by the walker |
| C23 `[[unsequenced]]`, `[[reproducible]]` (`p13_attributes.c:9-10`) | routine | **`unrecognized \| unsequenced \| - \| prefix \| std`** and `reproducible`: EDG has no semantics for them |
| `alignas(8) int al1` (`p13b_gnu_spellings.cpp`) | variable | `align \| alignas \| - \| prefix \| alignas`; `__attribute__((aligned(16)))` on a function (`p13b`): `align \| aligned \| - \| postfix \| gnu` and the type's `alignment: 16` |

**Attributes that leave something besides, or instead of, an entry.**
- `[[noreturn]]` in every spelling that EDG recognizes sets `does_not_return` on the routine's **function type**
  (`a_routine_type_supplement`, il_def.h:7540, printed `does_not_return: TRUE`) as well as leaving the entry.
- **`_Noreturn` (a specifier) and the `noreturn` macro of `<stdnoreturn.h>` leave NO attribute entry**; only the type flag:
  `p13_attributes.c`: `nr_spec` and `nr_macro` have `attributes: NULL` and the type has `does_not_return: TRUE` (il_display
  lines 2483, 2557, 2631 of the dump; also `nr_attr`). The C `[[noreturn]]` entry is named `_Noreturn`.
- Flags that duplicate an entry: `has_nodiscard_attribute` (routine, il_def.h:12582), `is_deprecated_or_unavailable`
  (routine/type/variable, :3569; il_display prints the flag line and then the entity's `attributes`, which belong to
  `source_corresp`), `is_pure` (:12591, for `pure` only; `const` leaves the entry only), `always_inline` (:12692; the
  prefix `[[gnu::always_inline]]` form, not `ai2`'s postfix one), `never_throws` (:12560, for `nothrow`).
- `__attribute__((noreturn))` in C23 with `<stdnoreturn.h>` included is macro-expanded to `__attribute__((_Noreturn))`
  (the entry's `position.orig_*` is in the header) and EDG records it `unrecognized | _Noreturn`, **without** the type flag:
  an attribute can be silently lost to a macro. `__attribute__((nodiscard))` (not a GNU attribute) is `unrecognized`.
- `__declspec(...)` is a syntax error in this configuration (`error #20: identifier "noreturn" is undefined`,
  `microsoft_mode: FALSE`), so it cannot occur in code that gets this far.
- Attribute entries that only function-region entities point to (locals, statements, the parameter variables) are not
  dumped by il_display; the walker, which follows the pointers in memory, prints them. That is how the back end can
  enumerate: walk every routine's function scope (variables, nested block scopes, statements) and read the `attributes`
  lists, which the walker does.
- Recognized-in-one-spelling attributes mean the back end must classify by `name` (and namespace), not by `kind`
  alone; `kind == unrecognized` entries still carry a usable `name`.

### P14: `#line` directives and linemarkers: **PASS**

`tests/il/m3/p14_line_directives.cpp` and `.c` (with the user header `p14_inc.inc`: the harness refuses a `.h` file in `tests/`).

```
 4  #include "p14_inc.inc"        (inc: from_header; then `#line 500 "renamed_in_header.h"`; after_line_in_header)
 5  int plain_before(int a)
 6  #line 100 "presumed.h"
 7  int after_line(int a)
 8  # 12 "marker.h" 3
 9  int after_system_marker(int a)
10  # 40 "tests/il/m3/p14_line_directives.cpp" 1
11  int after_return_marker(int a)
12  #line 70
13  int after_number_only(int a)
```
`seq-number-lookup-entry` records (il_display; `full_name` NULL marks a directive entry, il_def.h:150: "NULL for
entries generated by #line directives"):
```
lookup 1 4   line 1   tests/il/m3/p14_line_directives.cpp  full_name tests/il/m3/p14_line_directives.cpp
lookup 5 7   line 1   tests/il/m3/p14_inc.inc              full_name tests/il/m3/p14_inc.inc  inc
lookup 8 8   line 500 renamed_in_header.h                  full_name NULL  inc
lookup 9 10  line 5   tests/il/m3/p14_line_directives.cpp  full_name tests/il/m3/p14_line_directives.cpp
lookup 11 12 line 100 presumed.h                           full_name NULL
lookup 13 14 line 12  marker.h                             full_name NULL  sys
lookup 15 16 line 40  tests/il/m3/p14_line_directives.cpp  full_name NULL
lookup 17 17 line 70  tests/il/m3/p14_line_directives.cpp  full_name NULL
```
- Every `#line` or linemarker creates a `source-file` record (il_def.h:137) with `full_name: NULL` (:150), `name_as_written:
  NULL`, `file_name` = the presumed name (`presumed.h`), `first_line_number` = the new line, and a `seq-number-lookup-entry` for
  its sequence range. The record is a CHILD of the real file it appears in (`first_child_file` chain of the primary file
  lists `p14_inc.inc`, `presumed.h`, `marker.h`; the header's lists `renamed_in_header.h`), so the physical file of a
  directive is its parent. `#line 70` with no name reuses the current presumed name with a new record.
- A flag-3 linemarker (`# 12 "marker.h" 3`) makes the record `from_system_include_dir: TRUE` (:218): the code after it
  (`after_system_marker`, and in the C probe even the later `after_number_only` with `#line 70`) counts as **system** code:
  `seq_is_in_system_header` is true for it, and the walker neither reports nor counts it (the C probe's `end walk:` counts 3
  routines with bodies in non-system files and not the system one). A flag-1 linemarker back to the primary file's own name
  (`# 40 "tests/il/m3/p14_line_directives.cpp" 1`) makes later code look like the primary file with the wrong line numbers:
  `after_return_marker @ L40:5` and `after_number_only @ L70:5` are reported by the walker as primary-file routines at
  lines 40 and 70, which are not their physical lines (11 and 13). `seq_is_in_include_file` is false for them
  (`full_name == NULL` and the name equals the primary's: il.c:4856-4863).
- **Detection.** `walker.cpp:631-660` (`line_directive_warnings`) reads `il_header.seq_number_lookup_entries`, keeps those whose
  `source_file->full_name == NULL`, lies inside the primary file's sequence range and not inside a real include, and derives
  the directive's physical line with `conv_seq_to_physical_file_and_line` (il.c:4780). Output of the C++ probe:
  ```
  warning: line directive in the primary file at L6; code after it is attributed to presumed.h
  warning: line directive in the primary file at L8; code after it is attributed to marker.h
  warning: line directive in the primary file at L10; code after it is attributed to tests/il/m3/p14_line_directives.cpp
  warning: line directive in the primary file at L12; code after it is attributed to tests/il/m3/p14_line_directives.cpp
  ```
  It does **not** warn about the directive in the user header (`#line 500 "renamed_in_header.h"` at `p14_inc.inc:3`):
  the same test over the header's sequence range (any `source-file` with `full_name == NULL`, any parent) finds it, and
  that is the test for every verified file (the back end's own rule can be: a verified file contains a directive iff some
  `source-file` entry has `full_name == NULL` and its parent is that file).
- **Physical path vs presumed name.** For a real file `full_name` is "the actual file name" (il_def.h:150) as EDG built
  it from the command line or the include directory, **not canonicalized**: `tests/il/m3/p14_line_directives.cpp` and
  `tests/il/m3/p14_inc.inc` (relative to the working directory, joined to the includer's directory), absolute when the
  include directory was given absolutely (`/Users/.../include/cerastes.h`). `file_name` is the display form (the
  presumed name after a directive), `name_as_written` the text of the `#include` (`"p14_inc.inc"`). The physical file and
  line of any sequence number come from `conv_seq_to_physical_file_and_line(seq, &src_file, &line, &at_end)` (il.c:4780);
  `src_file->full_name` is the path to canonicalize (with `realpath` against the working directory), `conv_seq_to_file_and_line`
  gives the presumed pair.

### P15: shapes the encoder will walk: **PASS**

`tests/il/m3/p15_body_shapes.cpp`, `p15b_noreturn.cpp` (both `frontend-warning`).

- `(void)e;`: `stmk_expr` over `eok_cast tk=tk_unknown type=void result_is_not_used` (a `tk_unknown` cast to `void`; NOT
  compiler-generated) over the operand with `result_is_not_used` on the operand (`(void)a;`: `variable type=int a (param)
  is_lvalue result_is_not_used`; `(void)(a + b);`: the `eok_add is_parenthesized result_is_not_used`; `(void)sink(a);`: the
  `eok_call result_is_not_used`). Excerpt (`p15_body_shapes.cpp:12`, `(void)(a + b);`):
  ```
  stmk_expr @L12:3
    eok_cast tk=tk_unknown type=void result_is_not_used @L12:3 start@L12:3
      eok_add tk=tk_integer type=int is_parenthesized result_is_not_used @L12:12 start@L12:10
        variable type=int a (param) ol=int @L12:10 start@L12:10
        variable type=int b (param) ol=int @L12:14 start@L12:14
  ```
- A discarded expression statement without a cast (`a / 1;`, `a + b;`, `a;`) is `stmk_expr` directly over the node with
  `result_is_not_used` (`eok_divide ... result_is_not_used`, `variable ... is_lvalue result_is_not_used`); EDG warns
  `#174 expr_has_no_effect` for each. The operation is still in the IL, so its checks apply.
- `a / 0;`: `stmk_expr` over `eok_divide tk=tk_integer type=int result_is_not_used` with `constant type=int int:0
  [expr=NONE,is_simple_zero]`; EDG warns #39 `divide_by_zero` and #174, and does not fold or drop the operation.
- `int x = x;`: `stmk_decl`/`stmk_init` pair whose `dynamic-init` is `dik_expression` over `variable type=int x ol=int`, the
  variable being declared; EDG warns `#549 used_before_set`. The uninitialized read is visible to the encoder as an
  `enk_variable` whose variable is declared by the same statement.
- Return from a `[[noreturn]]` function (`p15b_noreturn.cpp`): EDG gives **warning #1305 `noreturn_function_does_return`**
  ("function declared with "noreturn" does return") at the `return` (`nr_return`, a `return;` inside), at the closing brace
  when the end is reachable (`nr_falls`, `nr_loop`) and at `return a;` of a non-`void` function (`nr_int`); it is a
  warning, the IL is the ordinary one (`stmk_return`, or the compiler-generated trailing `stmk_return` at `@-`), and code
  after a call to a noreturn function stays in the IL (`caller`: `nr_falls(a); a = 1;`). `nr_int` also shows that
  `[[noreturn]]` on a non-`void` function raises no diagnostic of its own in C++. Expectations: `frontend-warning
  noreturn_function_does_return @ L7, L8, L9, L10` in the probe's header.

### M3-4: facts found while writing the encoder

Each was checked against `il_def.h` and by the staged suite (`build/m3-staging/tests/scalar`, run through
`build/cerastes verify`); none contradicts an earlier probe.

- An **enumeration type** is a `tk_integer` with `variant.integer.enum_type` set (not a kind of its own); an
  enumerator's constant has an integer type but its original `expr` chain passes through a constant of the enum type.
  The encoder tests the flag (`vtype`, `constant`).
- A `break`/`continue` is a `stmk_goto` whose `a_label` has `break_label`/`continue_label` set (il_def.h a_label);
  the label statement the front end places for it is the same label (its flags are set, `compiler_generated` may not be):
  the encoder tells them from a user label by the flags.
- A variable length array leaves `is_vla`/`has_variably_modified_type` on the variable, a `stmk_vla_decl` and
  `stmk_set_vla_size`, and at the end of the scope a generated `stmk_expr` whose expression is `enk_vla_dealloc` plus
  an assignment to a return-value temporary: everything after the rejected declaration is the compiler's cleanup.
- A lambda expression is `enk_lambda`; its closure class has `extra_info->is_lambda_closure_class` and the call operator
  routine has `is_lambda_body`.
- `cerastes::old(x)` in an `ensures` is `eok_ref_indirect` over `eok_call` of the template instance `cerastes::old`
  whose argument is `eok_reference_to` of `eok_lvalue_adjust` of the parameter reference (`old(x + y)` has an
  `enk_temp_init` operand, which M2's checks reject).
- Internal errors of EDG are diagnostics of severity `es_internal_error`, counted as catastrophes, and exit with
  `RC_CATASTROPHE` (4), the same as a missing source file (`EXIT_ON_INTERNAL_ERROR`); only the text (`internal error
  #<n>`) tells them apart.
- `--error_output <file>` takes its value as a separate word; `f_error` (error.h) is the stream EDG's diagnostics, and
  the back end's, are written to.

### M3-4 (termination, decision 0013): IL facts the encoder uses

- `[[cerastes::decreases(e1, ..., en)]]` is one registered attribute with both targets (`"(X+)"`, flags `r|s:+i!`,
  `register_attributes.cpp`). After a function's parameter list it is on the routine's attribute list with
  `syntactic_location == al_post_func`, so `contracts_of(r)` returns it together with `requires` and `ensures`, in source
  order; the encoder tells them apart by `classify()` and must not treat a `decreases` as a postcondition. Its arguments are
  the `aak_expression` entries of `arguments_of(a)`, one per component; parameters are `enk_param_ref {k,0}`, as in
  `requires`. Before a `while` or `for` it is on the statement's own attribute list (`a_statement.attributes`), beside
  `invariant`. A `do`, range-based `for` or other loop that this version rejects carries its clauses too, and the
  statement is rejected once as such.
- In C++ a statement's `position` starts at its attributes, so for a loop with `CERASTES_INVARIANT(...)` or
  `CERASTES_DECREASES(...)` before it the position is the attribute's line; the keyword's line is found in the source text
  (`loop_keyword_position`). In C23 the position is the keyword's.
- The call graph is read from the bodies: every `eok_call` whose first operand is an `enk_routine` (`direct_callee`) is an
  edge from the routine being encoded to the callee; only callees with a body in a verified file (`body_scope`, and the
  body's position in a verified file) are edges, since a trusted callee is assumed to terminate. A function that reaches
  itself needs a measure.
- Positions: `a_source_position.seq` is the invocation (expansion) site of a macro, `orig_seq` and `orig_column` are the
  position of the text itself (basics.h:909-917): an argument token's own place, or a token of the macro's body at the
  `#define`. Nodes of user macro bodies are reported at the expansion site with a note at the `#define`.
- `an_asm_entry`: an `asm` statement of a body is an entry of a routine memory region (and a `stmk_asm`); a file-scope
  `asm(...)` is one the file-scope walk reports. Only the latter is rejected as a declaration.
- Attributes written on a type (`typedef int __attribute__((x)) T;`, or on the return type of a function declaration) are on the
  type entry's `source_corresp.attributes`, an entry with no declaration position of its own; the attribute's position
  decides whether it is in a verified file. `#pragma GCC system_header` makes a `source-file` record with the same
  `file_name` as its parent, `full_name == NULL` and `from_system_include_dir`; `#pragma clang system_header` is ignored
  by this EDG (warning #161).

## M4.1: control flow

Probes P13-P18 and Silicon spikes S1-S10 for decision 0014. Evidence files are under `build/m4.1/probes/` and
`build/m4.1/spikes/` (gitignored; produced with `scripts/cerastes-fe.sh --c23|--c++20 --il_display|--cerastes_walk <probe>`
and the pinned Silicon, run as the driver runs it). `il_def.h` lines are in `third_party/edg/src/il_def.h` at pin P.

### P13: generated `break` and `continue`

- `break` and `continue` are an `stmk_goto` (il_def.h:15817, `variant.label.ptr` at :16730) with `compiler_generated: TRUE`
  and the position of the `break;`/`continue;` text (`p13_while.c.c23.il`: `kind: stmk_goto`, `compiler_generated: TRUE`,
  `position.seq: 4 / column: 17`, `end_position.column: 22`). The label (`a_label`, il_def.h:13494) carries the flags
  `break_label` (:13511), `switch_break_label` (:13514, set in addition to `break_label` for a `break` of a switch) and
  `continue_label` (:13518); `source_corresp` has no name. Evidence: `label@..: break_label: TRUE` and
  `continue_label: TRUE` (p13_while.c.c23.il, the two `func-scope label@` entries; `p13_switch_loop.c.c23.il`: the label
  of a switch `break` shows `break_label, switch_break_label`).
- A label exists only when a jump uses it: an inner loop with a `break` and no `continue` has only a `break_label`
  (`p13_nested.c.c23.il`); a `for(;;)` left by `break` has no continue label (`p13_forever.c.c23.il`).
- There is no field linking a label to its construct. The generated `stmk_label` statements sit like this (identical in C23 and
  C++20, except that C23 wraps more things in `compiler_generated` blocks; see below):
  - **break label:** the immediately following sibling of the `stmk_while`/`stmk_for`/`stmk_end_test_while`/`stmk_switch`
    statement, in the same parent block (`p13_while.c.c23.il`: block `c7d0` holds `[stmk_while, stmk_label(break_label)]`;
    `p13_for.cpp.c++20.il`: the parent block holds `[stmk_for, stmk_label(break_label), stmk_return]`).
  - **continue label:** the last statement of the loop body block, or, when the body is wrapped (C23, a body that declares
    something: `p13_nested.c.c23.il`), the last statement of the compiler-generated block that wraps the body, right after it.
    It is before the `for` increment and before the `do` condition test.
  - **`for` increment:** not a statement of the body: it is `a_for_loop.increment` (il_def.h:15916, reached through
    `variant.for_loop.extra_info`); the loop initialization is `a_for_loop.initialization` (a generated block holding the
    `stmk_decl`/`stmk_init`). Evidence: `p13_for.c.c23.il` `stmk_for ... initialization: ... increment: expr-node` and the
    continue label after the last body statement.
  - **`do`:** `[stmk_end_test_while { loop_statement: block[..., stmk_label(continue_label)] }, stmk_label(break_label)]`;
    the condition `expr` is evaluated after the continue label (`p13_do.c.c23.il`).
  - **Switch:** `stmk_switch` followed by its `stmk_label` with `break_label` and `switch_break_label`; a `continue` inside
    the switch is a goto to the enclosing loop's `continue_label` (`p13_switch_loop.c.c23.il`: case 2's `continue` has
    `flags: continue_label`, the label statement is at the end of the loop body, after the `s = s + 1`).
- Finding the target: the flags decide the kind (`break_label` without `switch_break_label` = innermost enclosing loop;
  with it = innermost enclosing switch; `continue_label` = innermost enclosing loop), and the label statement's position
  confirms it (shape above). A label whose statement is not in one of those places should be rejected. C23 wraps statements
  in `compiler_generated` blocks that C++20 does not (`p13_for.c.c23.il` vs `p13_for.cpp.c++20.il`), so match on shape, not on
  parent depth.
- In C++20 conditions are `bool`-typed (`eok_lt ... [bool]`), in C23 `int`.

### P14: `do ... while`

- `stmk_end_test_while`: `expr` = the standardized condition (il_def.h:16599), `loop_statement` = body (:16648). There is no
  keyword position and no `while`-keyword position field (`grep` of il_def.h finds none). `position` is the statement start; in
  C++20 it starts at the first attribute (`p14_do.cpp.c++20.walk`: macro form `stmt stmk_end_test_while @ L6:3`, explicit
  `[[...]]` on its own line `@ L15:3` = the attribute line), in C23 it is the `do` keyword (`p14_do.c.c23.walk`: `@ L6:74` for the
  macro form, `@ L17:3` for the explicit form). `end_position` is the end of the `;`.
- Attributes: `[[cerastes::invariant(..)]]` and `[[cerastes::decreases(..)]]` before `do`, by macro or written out, are on the
  statement's attribute list and print in the walker as `stmt stmk_end_test_while ... attr [[cerastes::invariant]] @ L..`
  (C and C++, `p14_do.cpp.c++20.walk`, `p14_do.c.c23.walk`). No EDG diagnostic. Today `cerastes verify` rejects the statement once
  with `cerastes-unsupported_statement` at the `do` keyword (`build/cerastes verify p14_do.c --std=c23`, line 6 and line 17).
- Constant conditions: `do { } while (0)`, `while (false)` and `while (1 - 1)` give `expr.kind == enk_constant` with
  `integer_value 0`, never an operation (`p14_do.cpp.c++20.il`, `p14_do.c.c23.il`). The original expression is kept on the
  constant: `a_constant.expr` (il_def.h:4782). C++20 for `while (1 - 1)`: the condition constant (type `bool`, `0`) has
  `expr = eok_bool_cast(...)` whose operand is the `int` constant `0` whose `expr` is `eok_subtract(1, 1)` (`p14_do.cpp.c++20.il`
  around `constant@76279a0f70`); C23: the `int` constant 0 has `expr = eok_subtract(1, 1)`. A C++20 literal `0` also has
  `expr = eok_bool_cast(0)`; `false` (C++20: type `bool`; C23: type `bool`, value `0U`) and a C23 literal `0` have no `expr`. So "constant zero after folding" is `enk_constant` with an integer value 0
  and the original text is recoverable only through `a_constant.expr`.

### P15: `switch`

- `stmk_switch`: `expr` = the scrutinee, `variant.switch_stmt.body_statement` (il_def.h:16717), `extra_info` =
  `a_switch_stmt_descr` (:16305: `cases` in source order, `default_case`, `sorted_cases`). Each `stmk_switch_case`
  (:16703) has `switch_statement` (the owning switch) and `extra_info` = `a_switch_case_entry` (:16251: `case_value` (NULL
  for default), `range_end`, `next`, `position`, `reachable_by_fall_through`).
- **The scrutinee is integer-promoted by EDG**: `eok_cast(var c)[int]` for `char`, `unsigned char`, `bool`/`_Bool` and `short`;
  no cast for `int`, `unsigned int`, `long long`, `unsigned long`; `s + 1` for a `short` is `eok_add(eok_cast(s)[int], 1)`
  (`p15_switch.c.c23.il`, `p15_switch.cpp.c++20.il`).
- **Case values are converted to the promoted scrutinee type**: `case 'a'` is `97 : int`; `case 255`/`case -1` on
  `unsigned char` are `255 : int`/`-1 : int` (unreachable values, no diagnostic); `case 65535` on a `short` is `65535 : int`;
  `case -1` on `unsigned int` is `4294967295U : unsigned int` with warning "integer conversion resulted in a change of sign" in C23
  (and then a duplicate with `case 4294967295u` is the EDG error "case label value has already appeared"), and an EDG error
  "invalid narrowing conversion" in C++20 (`p15_neg.cpp`). `long long`: `5000000000LL : long long`, `-2LL`; `unsigned long`:
  `1UL : unsigned long`. The `case_value` constant's `type` is always the promoted scrutinee type. Duplicate case values and a
  second `default` are EDG errors in both languages (`p15_neg.cpp`).
- Body shape: `case`/`default` are sibling statements in the body block, the "labelled" statements follow as plain siblings
  (`case 1: case 2: return 1;` is two consecutive `stmk_switch_case`; `p15_switch.c.c23.il`, `f_int`). A case that follows
  fallthrough code has `reachable_by_fall_through: TRUE` (cases after a `return`/`break` do not).
- The `switch_break_label` is generated only when a `break` uses it (a switch with no `break` has no label statement; the
  `f_char` function in `p15_switch.c.c23.il` has none).

### P16: switch condition declaration and init-statement (C++)

- `switch (int y = e)`, `switch (int z = a; z + 1)` and `switch (a; a)` all give `stmk_switch` with `expr.kind == enk_condition`
  (il_def.h:13593, `variant.condition` :15445); the ordinary `switch (a)` gives a plain expression, never an `enk_condition`
  (`p16_switch_decl.cpp.c++20.il`, `p15_switch.cpp.c++20.il`). In the declaration form the `enk_condition` has `dynamic_init` set
  and `initialization: NULL`; in the init-statement forms `initialization` is a `stmk_decl` or `stmk_expr` statement with
  `dynamic_init: NULL`. A rejection keyed on `expr.kind == enk_condition` covers all three and needs no more.

### P17: nested case labels, case ranges, skipped declarations, unreachable code

- **Nested case labels** are accepted by EDG without diagnostic in C23 and C++20 (`p17_duff.c`). A case inside a `while` body in
  the switch (`duff`), inside a nested block (`inblock`) or inside an `if` (`inif`) has `parent` = that inner block, not the switch's
  body block; `stmk_switch_case.switch_statement` still points at the switch. Directly in the switch body block: the case's
  `parent` equals `body_statement` (`p17_duff.c.c23.il`). The check "case marker directly in its switch body" is a parent test.
- A statement before the first label (`c = 5;` then `case 0:`) is accepted; EDG warns `#111-D statement is unreachable`
  (`scripts/cerastes-fe.sh --c23 --display_error_number p17_duff.c`).
- **GNU case ranges are accepted silently in C23 and C++20 (the cerastes configuration is GNU-emulating)**: `case 1 ... 3:` has
  `case_value 1` and `range_end 3` (`a_switch_case_entry.range_end`, il_def.h:16262; `p17_range.c.c23.il`). Reject them on
  `range_end != NULL`.
- **Skipped declarations: warning in C, and in C++ only for a non-trivially initialised object**: `int x = 5;` between `case 0:` and a
  later case label gives `warning #546-D: transfer of control bypasses initialization of: variable "x"` in both C23 and C++20
  (`p17_skip.c`, `p17_skip.cpp`; rc 0, so the plan's "C++ already refuses it" is wrong for scalars); `int y;` without an
  initializer gives no diagnostic; a class with a constructor gives `error #546` in C++20 (`p17_skip2.cpp`). Both modes need
  cerastes's own rejection of a declaration that a later case label jumps over.
- Unreachable code after `return` or after `while (1) { return 1; ... }` gives `warning #111-D` only.

### P18: `[[fallthrough]];`

- A `stmk_empty` statement with an attribute list (`stmt stmk_empty @ L6:7 attr [[fallthrough]]`, `p18_fall.cpp.c++20.walk`;
  `attribute kind ak_fallthrough`, il_def.h:2978, `an_attribute.name` "fallthrough", `namespace_name` NULL). `[[clang::fallthrough]]` and
  `[[gnu::fallthrough]]` appear as `attr [[clang::fallthrough]]` and `attr [[gnu::fallthrough]]` on the same `stmk_empty`; in C23
  `__attribute__((fallthrough));` also prints as `attr [[fallthrough]]` (`p18_fall.c.c23.walk`). It is on the same statement in a
  template's prototype and its instance (`p18_fall.cpp.c++20.walk`, routine `g`). No EDG diagnostic in either language. The encoder's
  attribute allow-list (`allowed_attribute_name`, `encode.cpp:56`) is name-only.

### Silicon facts (spikes S1-S10, `build/m4.1/spikes/`, Silicon 6b540aa4, Z3 4.16.0)

- A `goto` out of a loop that has an invariant and a `decreases`, to a label after the loop or to `l_end`, verifies and the exit
  paths are known separately (S1a, S1b: the assert true only on the normal exit fails at that assert).
- Labels inside a loop body (the `continue` target) work, including gotos to the same label from a nested loop or from the
  dispatch region of a switch (S2a, S4, S6, S10). A continue that skips the `for` step is `termination` at the `decreases`
  line (S2b, S6b: `Loop might not terminate. Termination measure might not decrease or might not be bounded`, reported at
  the `decreases` expression, not at the `while`); a continue path that breaks an invariant is `invariant.not.preserved` at the
  invariant line (S3).
- `while (true)` loops left only by `goto` verify; in the `do` shape (body; label; assert; `if (!c) goto brk`) entry, preserved and
  termination twins fail with the usual errors at the usual lines (S7a-S7d); a negative measure in the only/last iteration is
  accepted (S7e). The invariant of such a loop is assumed at the start of an arbitrary iteration, so for `do { i++ } while (i < n)`
  it must say `i == 0 || i < n`-style things to prove facts after the loop (S7a).
- `do ... while (0)` in the loop shape with `decreases 0` passes the termination check but loses every fact about variables assigned in
  the loop (S7f3: `ensures res == 1` fails); lowered **without a Viper loop** (body; continue label; break label) it keeps them (S7f2).
- A `while (true)` with no exit: Silicon does not report code after it (S8c, S8d: `assert false` after it verifies in a method with no
  termination checking, and is not reported in a method with one); the loop itself fails termination (S8b, S8c).
- Switch dispatch with a default in the middle and fallthrough verifies (S9a, twin S9b fails at the postcondition).
- A jump into a loop body (a case label inside a loop, S9c) makes **Silicon throw `ClassCastException`** (not a verdict);
  a label inside an `if` branch as the target of a jump from outside is accepted (S9d). The lowering must never produce either by
  construction.

## M4.2a: goto, noreturn, assert

Probes P19-P22 and Silicon spikes S11-S13 for decision 0015. Evidence files are under `build/m4.2a/probes/` and
`build/m4.2a/spikes/` (gitignored; produced with `scripts/cerastes-fe.sh --c23|--c++20 --display_error_number --il_display
|--cerastes_walk <probe>` and the pinned Silicon, run as the driver runs it). Probe names below are files in that directory
(`p19_goto.c`, with `.cpp` twins; `<probe>.<c23|c++20>.il` is the il_display output). `il_def.h` lines are in
`third_party/edg/src/il_def.h` at pin P. The statement and expression trees quoted are condensed from il_display by
`build/m4.2a/tools/tree.py` (positions are `seq:column`).

### P19: user `goto` and labels

- A user `goto L;` is `stmk_goto` (il_def.h:15817) with `compiler_generated` unset, `label` the `a_label` (il_def.h:13494) and
  position 3:14-3:20; the label is `stmk_label` (:15818) covering only `L:` (5:1-5:2). The labeled statement is the **next
  sibling**, not nested (`p19_goto.c.c23.il`: `stmk_label @5:1-5:2` then `stmk_expr @6:3`; identical in C++20). `a_label.exec_stmt`
  (:13545) points at the `stmk_label`; `source_corresp.name` is "L", `decl_position` 5:1. No break/continue flags.
- The `goto` of `if (c) goto L;` is always inside a compiler-generated then-block (both modes). In C23 the `if` itself is inside a
  compiler-generated block too (P20).
- **Gotos out of loops and switches generate nothing**: a `goto` out of `while`, `for`, `do`, a `switch` case and
  `do { goto E; } while (0)` is a plain `stmk_goto` (`p19_out.*.il`); each label is the next sibling after the loop (C23: after the
  generated wrapper block) and, for a switch, after the switch's own generated break label. EDG warns `#111-D` for the
  unreachable statement after `do { goto E; } while (0)`.
- **A declaration after a label** (`L: int y = 2;`) is accepted in both modes with no diagnostic; `stmk_decl` and `stmk_init`
  are siblings after the `stmk_label` (`p19_decl.*`). **A label before `}`** (`L: }`): no diagnostic in C23, `warning #127-D:
  expected a statement` in C++20; no `stmk_empty` either way, and EDG appends a compiler-generated `stmk_return` (`p19_end2.*`).
- **Attributes on a label are on the label entity, not on the statement**: `[[maybe_unused]] L:` and `L: __attribute__((unused))` are
  accepted in both modes; the `stmk_label` has `attributes: NULL`, the label entity has `maybe_unused: TRUE` and
  `source_corresp.attributes` set (`p19_attr.*`, `p19_attr2.*`). An unrecognized `[[foo::bar]] L:` survives there (`warning
  #2803-D` for the namespace; the walker prints `attr [[foo::bar]] on label f::L @ L4:5`, `p19_attr3.*`); `[[gnu::cold]] M:` is
  accepted. The encoder's `check_attributes_statement(s)` reads `s->attributes` and so never sees them: the allow-list must run over
  `a_label.source_corresp.attributes`.
- **GNU local labels** (`__label__ L;`) are accepted with no diagnostic and no IL difference (`p19_locallabel.*`). **Computed goto**
  (`void *p = &&L; goto *p;`) is accepted; the statement is `stmk_assigned_goto` (il_def.h:15856) with `expr = var p : void *`
  (`p19_computed.*`).
- Diagnostics: a label never referenced is `warning #177-D: label "L" was declared but never referenced`; a `goto` to an undefined
  label is `error #114: label "L" was referenced but not defined` (both modes; `p19_unref.*`, `p19_undef.*`).

### P20: scopes and generated blocks

- **C23 wraps every `if`, `for`, `while`, `do` and `switch` statement in one compiler-generated block holding just that statement**;
  C++20 does not (`p20_case.c.c23.il` routine h vs `p20_case.cpp.c++20.il`). In both modes a brace-less then, else or loop body is a
  compiler-generated block, `else if` is a generated else-block (C23: holding a generated wrapper holding the `if`), and a `for`
  init-statement is a generated block in `a_for_loop.initialization`.
- **A declaration directly after `case` has no generated block**: `case 1: int y = 1; return y;` leaves `stmk_decl`, `stmk_init`
  as siblings in the switch body, in both modes (`p20_case.*`, routine f); likewise after a label. The generated blocks that can hold
  a declaration are the `for` init-statement (handled on its own by the encoder) and the branch block of `if (c) int y = 1;` (error
  #525 in C23; C++20 accepts it with only #177): there the block's scope is the branch, which is the C/C++ scope. The C23 wrappers
  never hold a declaration. So **flattening generated blocks (`encode.cpp:1424`) gives a VIR block equal to the C scope** for every
  shape probed; `declared_variable` (`encode.cpp:1311`) only matters for the `for` init.
- **A jump past an initialization** (`if (x > 0) goto L; int y = 1; L: return y;`): C23 `warning #546-D: transfer of control bypasses
  initialization of: variable "y"`, positioned at the `goto`; C++20 `error #546`, even when `L:` labels only a null statement
  (`p20_skip.*`, `p20_skip3.*`). `int y;` without an initializer gives no #546 in either mode (`p20_skip2.*`). A case label jumping
  over a declaration is a warning in both modes (P17).

### P21: functions that do not return

- **`does_not_return`** (`a_routine_type_supplement`, il_def.h:7540, printed on the `tk_routine` type) is TRUE for `[[noreturn]]`,
  `[[gnu::noreturn]]`, `__attribute__((noreturn))` and `_Noreturn` in both modes, and for `[[_Noreturn]]` in C23 only. The routine's
  attribute list (`routines.py`, `p21_nr.*.il`):

  | spelling | C23 | C++20 |
  |---|---|---|
  | `[[noreturn]]` | kind noreturn, family std, no namespace | same |
  | `[[gnu::noreturn]]` | kind noreturn, namespace gnu | same |
  | `__attribute__((noreturn))` | kind noreturn, family gnu | same |
  | `_Noreturn` | **no attribute entry** | **no attribute entry** (accepted silently) |
  | `[[_Noreturn]]` | kind noreturn, name `_Noreturn` | `kind unrecognized`; `does_not_return` NOT set; no diagnostic |

  So cerastes reads `does_not_return` of the routine type (the check at `encode.cpp:404` already does), never attributes.
- **Redeclarations**: `[[noreturn]]` on a later declaration is `error #1846: attribute "noreturn" did not appear on original
  declaration` (both modes, `p21_redecl.*`); a plain redeclaration after a `[[noreturn]]` or `_Noreturn` one keeps one routine entity
  with `does_not_return` TRUE (`p21_redecl2.*`).
- **Warnings**: `#1305-D: function declared with "noreturn" does return` at the `return` keyword of `return;` and at the closing
  brace when control falls off the end (`p21_nr.*`, functions g1-g4, both modes); `[[noreturn]] int f1(void)` gives `#3271-D:
  "noreturn" function has non-void return type` in C23 only.
- **The C library** (`p21_decls.*`): `abort` is in the SDK's `_abort.h`, `exit`, `quick_exit` and `_Exit` in `_stdlib.h` (both under
  `usr/include`, `from_system_include_dir`). All have `does_not_return` TRUE, `name_linkage C` also in C++, `sc_extern`, attributes
  `__noreturn__` (and `__cold__` for `abort`); parameter lists `void()` / `void(void)` for `abort` and `void (int)` for the others.
  `std::exit` and `std::abort` call **the same routine entities** as `::exit` and `::abort` (one address `routine@7ca41f5dd0` for
  `exit` in all three spellings, `p21_decls.cpp.c++20.il`).
- **Look-alikes** (`p21_builtin.*`): `__builtin_trap` and `__builtin_unreachable` are ordinary `eok_call`s of an `enk_routine` with
  `builtin_function_kind` set (il_def.h:12312), `compiler_generated`, no declaration position, `does_not_return` TRUE; C23
  `unreachable()` (`<stddef.h>`) expands to `__builtin_unreachable` (the same entity); `std::terminate` is a system routine with
  `does_not_return` TRUE (`__noreturn__`, `noexcept`). `__assert_rtn` and `__builtin_expect`: see P22 (`__assert_rtn` has the flag,
  `__builtin_expect` does not).

### P22: `assert`

- `<assert.h>` (C23) and `<cassert>` (C++20) produce no diagnostic; the macOS expansion is
  `(__builtin_expect(!(e), 0) ? __assert_rtn(__func__, __FILE_NAME__, __LINE__, #e) : (void)0)` (SDK `assert.h:72`), and
  `((void)0)` under `NDEBUG` (`p22_assert.*`, `p22_ndebug.*`).
- **Tree** of `assert(x > 0);` (`p22_assert.c.c23.il`): `stmk_expr` of
  `eok_question[void]( eok_call[long](__builtin_expect, eok_cast[long](eok_not[int](eok_gt[int](var x, const 0))), const 0L),
  eok_call[void](__assert_rtn, array_to_pointer(var <__func__>), <file-name literal>, const 5:int, <text literal "x > 0">),
  eok_cast[void](const 0:int) )`. `__func__` is a compiler-generated, unnamed, static, constexpr variable local to the routine
  (`source_corresp.enclosing_routine`, no name, `init_kind initk_static`, constant "g", type `const char[2]`). `assert(f(x) > 0)`
  has an `eok_call[int]` of `f` as the left operand of `>`. `__assert_rtn` (system `_assert.h`, C linkage, `void (const char *,
  const char *, int, const char *)`) has `does_not_return` TRUE (`__noreturn__`, `__cold__`); `__builtin_expect` is a builtin
  routine of type `long (long, long)` without the flag.
- **C++20 differences**: the condition is wrapped as `eok_bool_cast[bool](eok_call[long](__builtin_expect ...))`; `!` and `>`
  are `bool`-typed; `__builtin_expect` is `throw()`; the file-name literal is `const char[15]`.
- **Positions**: every node's `position` is the macro invocation (`2908:3`, the statement start). Only
  `expr_range.start.orig_seq/orig_column` (EXTRA_SOURCE_POSITIONS_IN_IL) tells user text from macro text: the user's `x`, `0`, the
  `>` node and the stringified text literal have `orig` on the user's line (`2908:10`, `2908:14`, `2908:10`, `2908:10`); the `!`,
  `__builtin_expect`, `0L`, `__assert_rtn`, `__func__`, `__LINE__` (`5`) and the `(void)0` have `orig` in assert.h (`2862:...`); the
  file-name literal has `orig 2848`. Under `NDEBUG` the statement is `eok_cast[void](const 0)` only.
- Today `build/cerastes verify` on a file with `assert` reports `cerastes-call_without_contract` for `__builtin_expect` and
  `cerastes-unsupported_type` for `void`, at the line of the `assert`, in both modes; with `NDEBUG` the file verifies
  (`p22_assert.*`, `p22_ndebug.c`).

### Silicon facts (spikes S11-S13, `build/m4.2a/spikes/`, Silicon 6b540aa4, Z3 4.16.0)

- **S11 (`s11a`, `s11b*`, `s11c*`)**: a `goto` from an inner loop to a label after the outer loop verifies (`s11a.vpr`); the twin
  asserting a fact true only on the normal exit fails at that assert (`s11a_twin.vpr@30`). A `goto` from a dispatch inside a loop
  body to a later label of the same body verifies (`s11b.vpr`); the twin whose goto path breaks an invariant is `Loop invariant
  res >= 0 && res <= i might not be preserved. Assertion res <= i might not hold. (s11b_twin.vpr@11.15--11.35)`, and the twin whose
  label is after the step is `Loop might not terminate ... (s11b_twin2.vpr@12.15--12.20)` (the `decreases` line). A `goto` out of
  straight-line `DoOnce` code to a later label verifies, and a fact true only on the normal path fails after the label
  (`s11c.vpr`, `s11c_twin.vpr@18`).
- **S12 (`s12a`, `s12b`, `s12c`, `s12c2`)**: a bodyless `method abort_() requires true ensures false decreases _` called inside a
  loop with a measure and from a method with `decreases` verifies, and an `assert false` after the call verifies (`s12a.vpr`); without
  the call it fails (`s12a_twin.vpr@19`). A bodied `ensures false` method whose returns are `assert false` verifies when each path
  ends in such a call (`s12b.vpr`). With a reachable return Silicon reports **only** the explicit `assert false` (`Assert might fail.
  Assertion false might not hold. s12c.vpr@13.12--13.17`), not the postcondition; without the explicit assert it reports `Postcondition
  of die might not hold. Assertion false might not hold. (s12c2.vpr@8.11--8.16)`.
- **S13 (`s13`, `s13b`)**: `assert b != 0; assert (a \ b) >= 0` verifies with `b > 0`; with `b` possibly 0 Silicon reports three
  errors: `b != 0` (line 8), a cascade `a \ b >= 0` at the next line (9) and, in the second method, the genuinely false `a \ b >= 1`
  (21). After a call of `abort_()` every assert holds (`assert 1 == 2` verifies, `s13.vpr`).

## M4.2b1: named constants and case values

Probes P23-P24 and Silicon spikes S14-S15 for decision 0017 (named constants, case values in a path-independent prologue,
the `do … while` condition). Evidence is under `build/m4.2b1/probes/` and `build/m4.2b1/spikes/` (gitignored), produced
with `scripts/cerastes-fe.sh --c23|--c++20 --display_error_number --il_display|--cerastes_walk <probe>` (output
`<probe>.<c|cpp>.<c23|c++20>.il` / `.walk` / `.err`) and the pinned Silicon, run as `run_tests.py` runs it (`java -Xss128m -jar
silicon.jar --z3Exe z3 --numberOfErrorsToReport 0 <file>`). The trees quoted are condensed from il_display by the scratch tools
`build/m4.2b1/tree.py` and `tree2.py` (`vars`, `fn <routine>`, `cases`, `dik`; positions omitted; `{expr=...}` is the folded
constant's `expr` chain). `il_def.h` lines are in `third_party/edg/src/il_def.h` at the pin. Probe files: `p23_g.cpp`
(C++ globals), `p23_gb.c` (the C globals that EDG accepts; `p23_g.c` is the first attempt, refused), `p23_use.c|cpp`,
`p23_z.c|cpp`, `p23_zero.c|cpp`, `p24_case.c|cpp`, `p24_ty.c|cpp`. Both modes run on every probe unless a construct is invalid in one.

### P23: named constants (namespace scope and locals)

**Variable fields** (a_variable, il_def.h:11215-11874): `type` :11227, `storage_class` :11251, `is_constexpr` :11533,
`has_explicit_initializer` :11622, `constant_valued` :11710 ("Only set in C++"), `is_thread_local` :11720, `init_kind` :11745,
`initializer` :11752 (union an_initializer :11034; `.constant` for `initk_static`, `an_init_kind` :11014). Per variable, at
namespace scope, `tree.py vars` on `p23_gb.c.c23.il` and `p23_g.cpp.c++20.il`:

| source | C23 | C++20 |
|---|---|---|
| `const int g = 40;` | `initk_static`, constant `int 40`, `sc_unspecified`, linkage C (external) | `initk_static`, `int 40`, `constant_valued`, **`sc_static`**, linkage internal |
| `constexpr int c = g + 2;` | **not accepted** (below) | `initk_static`, `is_constexpr`, constant `int 42 {expr=eok_add(const 40 {expr=var g}, const 2)}` |
| `const short s = 7;` | static, `short 7 {expr=eok_cast[short](const int 7) gen}` | same |
| `const bool b = 1;` | static, `bool 1U {expr=eok_bool_cast[bool](const int 1) gen}` (the value prints `1U`) | static, `bool 1 {expr=eok_bool_cast...}` |
| `const unsigned u = -1;` | static, `unsigned 4294967295U {expr=eok_cast(const int -1 {expr=eok_negate(const 1)}) gen}`; warning #68-D | same |
| `const int sh = 1 << 40;` | static, `int 0 {expr=eok_shiftl(const 1, const 40)}`; warning #63-D | **static** (not dynamic), same constant; warning #63-D |
| `const signed char sc = (signed char)300;` | static, `signed char 44 {expr=eok_cast[signed char](const int 300)}`; no diagnostic | same |
| `const int ov = 2147483647 + 1;` | static, `int -2147483648 {expr=eok_add(const 2147483647, const 1)}`; warning #61-D | **`initk_dynamic`**, no constant, no `constant_valued`; warning #61-D |
| `const int z;` | **`initk_none`**, `sc_unspecified`, no `has_explicit_initializer` (same as `int w;`); never `initk_zero` | **error #257** (const variable requires an initializer) |
| `extern const int e;` | `initk_none`, `sc_extern` | same (linkage C++) |
| `extern const int d = 5;` | static `int 5`, `sc_unspecified`; no diagnostic | static `int 5`, `constant_valued`, `sc_unspecified`, linkage C++; no diagnostic |
| `const int self = self + 1;` | n/a (C: not probed) | `initk_dynamic`, no constant, no diagnostic |
| `int f(void); const int fc = f();` | **error #59** (function call is not allowed in a constant expression) | `initk_dynamic`, no constant |
| `volatile const int vc = 1;` | static, constant `int 1`, type `const volatile int` | static, `int 1`, but **no `constant_valued`** |
| `inline constexpr int ic = 3;` | n/a | static, `is_constexpr`, `int 3`, `sc_unspecified` |
| `static const int st = 4;` | static `int 4`, `sc_static`, linkage internal | same |
| `_Thread_local` / `thread_local const int tl = 5;` | static `int 5`, **`is_thread_local: TRUE`** | static `int 5`, `constant_valued`, `is_thread_local: TRUE` |
| `sys_nerr` (`<stdio.h>`, `extern const int`) | `initk_none`, `sc_extern`, linkage C, declared in the SDK | same |

Further, `p23_z.c.c23.il`: `const int zz; const int zz = 3;` is **one** variable (`initk_static`, constant 3, `decl_position` at the
defining line 5, a use folds to `const int 3 {expr=var zz}`); `const int ex2 = 6; extern const int ex2;` stays `initk_static`
6; `int w;` and a tentative `const int z;` stay `initk_none` and their uses are a plain `var`. `p23_zero.*`: `const int zb =
{0};` and C23 `const int ze = {};` are static with constant 0 and **no `expr`**; C++ `const int zi{};` is static 0 with no `expr`;
C++ `const int zv = int();` is static 0 with `{expr=enk_temp_init}` (il_def.h:13583). `initk_zero` was not produced by
any construct above (the zero-value shape is `initk_none` in C, an error in C++).

**C23 `constexpr` is not accepted by this EDG configuration**: `constexpr int c = g + 2;` gives `warning #77-D: this declaration has
no storage class or type specifier` and `error #65: expected a ";"` at the `int` (`p23_g.c.c23.err`), and the later use `c` is
`error #20: identifier "c" is undefined`. So `constexpr` globals and locals exist only in C++20 here; in C23 the constants are
`const`. (Not probed: another `--std` spelling for C23 `constexpr`.)

**Uses (folded)**, `tree.py fn` on both modes (`p23_gb.c.c23.il`, `p23_g.cpp.c++20.il`): every use of a variable whose
initializer is a constant is an `enk_constant` whose constant carries the variable's value and whose `expr` is an
`enk_variable` node of the variable, and a larger folded expression is **one** constant whose `expr` is the unfolded tree with
those leaves: `return x * g;` is `eok_multiply(var x, const int 40 {expr=var g})`; `return s + 1;` is `const int 8
{expr=eok_add(const int 7 {expr=eok_cast[int](const short 7 {expr=var s}) gen}, const int 1)}`; `return b;` (C23) is `const int 1
{expr=eok_cast[int](const bool 1U {expr=var b}) gen}`; `return u;` as `(int)u` is `const int -1 {expr=eok_cast[int](const unsigned
4294967295U {expr=var u})}`; `return ov;` (C23) is `const int -2147483648 {expr=var ov}`; `return sh;` is `const int 0 {expr=var sh}`.
Promotion casts sit **inside** the chain, between the variable and the folded value. Un-folded uses are a plain `var`:
`e`, `sys_nerr`, `vc` (both modes), `z` (C), and in C++ also `ov`, `self`, `fc` (dynamic). In all 21 folded uses probed, the use's
value equals the variable's initializer constant converted by the casts in the chain; no counterexample was found (not a proof).
`local_expr_ref` (il_def.h:2788) was never observed: no constant in any probe printed it (unconfirmed that il_display would print it).

**Uses in annotations** (`p23_use.c|cpp`, `--il_display` and `--cerastes_walk`): a global `g` used in `CERASTES_REQUIRES(x < g)`,
`ENSURES(.. r == x + g ..)`, `CHECK(g == 40)`, `INVARIANT(i <= g)`, `DECREASES(g - i)`, a loop condition and a `return` gives 7 `enk_variable`
nodes of `g` in the IL, **all 7 referenced as the `expr` of a folded constant 40**, in both modes (Python check over `p23_use.*.il`:
`g-nodes 7 constants linked 7 ['40']`); the contract nodes carry the attribute's position (L7:5, L8:5), the statement-attribute ones 3:3.
The walker prints them as `const 40 type=int @ L7:27` (it does not print the `expr`), `s` as `op eok_cast ... const 7 type=short`, `u` as `const 4000000000
type=unsigned int`.

**Locals** (`p24_case.c|cpp`, `tree2.py dik`): `const int k = 5;`, `constexpr int n = 3;` (C++ only), `const int kc = (signed char)300;`, `const int ks =
1 << 40;`, `const int g0 = 0;` are **`init_kind initk_dynamic`** variables with **no `constant` field** in both modes, `sc_auto`; the value is in the
`stmk_init`'s `dynamic_init` (a_dynamic_init), `kind dik_constant` (il_def.h:3969), `variable` k, `constant` `int 5`; `kc` is `int 44 {expr=eok_cast[int](const signed
char 44 {expr=eok_cast[signed char](const int 300)}) gen}` and `ks` `int 0 {expr=eok_shiftl(const 1, const 40)}`. `constant_valued` and `is_constexpr` are
set in C++ only (`k`: `constant_valued`; `n`: both). A use of a local `k` folds like a global's: `case_value = int 5 {expr=var k}`.
EDG accepts `case k`, `case n`, `case kc`, `case ks` in **both modes**: in C23 `case N` and `case k` on a non-constexpr `const` give `warning #1901-D: use of a const
variable in a constant expression is nonstandard in C` (also for `case N + 1` and `case NC`), accepted; C++ gives no diagnostic (only #63-D for the shift).

### P24: case values and `do … while` conditions

**Case values** (`tree2.py cases`; the entry is `a_switch_case_entry`, il_def.h:16251, `case_value` :16259): for `const int N = 8;`, `const int NC = 3;` and the locals above,
identical in C23 and C++20 (`p24_case.*`, `p24_ty.*`):

- `case N:` is `case_value int 8 {expr=var N}`; `case N + 1:` is `int 9 {expr=eok_add(const int 8 {expr=var N}, const int 1)}`; `case NC:` (C++: `constexpr`) `int 3 {expr=var NC}`;
  `case k:` `int 5 {expr=var k}`; `case n:` `int 3 {expr=var n}`; `case kc:` `int 44 {expr=var kc}`; `case ks:` `int 0 {expr=var ks}`. The chain always reaches the variable.
- **The case value's type is the promoted scrutinee type** with the conversion in the chain: for `signed char x`, `case N` is `int 8 {expr=var N}`; for `unsigned x` it is
  `unsigned int 8U {expr=eok_cast[unsigned int](const int 8 {expr=var N}) gen}`; for `long x`, `long 8L {expr=eok_cast[long](const int 8 {expr=var N}) gen}`;
  a plain `case 3` is `unsigned int 3U {expr=eok_cast[unsigned int](const int 3) gen}` there.

**`do … while (c)`** (the condition is the `expr` of `stmk_end_test_while`, il_def.h:15825): for `Z0`/`g0`/`k0`/`n0` = 0 (global const, local const, local constexpr) and `N - N`:

| condition | C23 | C++20 |
|---|---|---|
| `while (Z0)` global const 0 | `const int 0 {expr=var Z0}` | `const bool 0 {expr=eok_bool_cast[bool](const int 0 {expr=var Z0}) gen}` |
| `while (g0)` / `(k0)` local const 0 | `const int 0 {expr=var g0}` | `const bool 0 {expr=eok_bool_cast(const int 0 {expr=var g0}) gen}` |
| `while (n0)` constexpr local | n/a | `const bool 0 {expr=eok_bool_cast(const int 0 {expr=var n0}) gen}` |
| `while (N - N)` | `const int 0 {expr=eok_subtract(const int 8 {expr=var N}, const int 8 {expr=var N})}` | `const bool 0 {expr=eok_bool_cast(const int 0 {expr=eok_subtract(...)}) gen}` |
| `while (0)` | `const int 0` with **no `expr`** | `const bool 0 {expr=eok_bool_cast[bool](const int 0) gen}`, the inner `const int 0` has no `expr` |

So the condition is always a folded `enk_constant` 0 (never an `enk_variable`), and a variable it names is found only by following the `expr` chain through
operations and casts (C++ adds the generated `eok_bool_cast` on top). `while (0)` is distinguished by an inner constant whose `expr` is NULL.

**Audit additions (2026-10-21, Opus audit of 0017).**

- **C `const int self = self + 1;` is refused by the front end**: `error #28: expression must have a constant value` with `note #2689-D: the value of variable "self"` (`/tmp/audit0017/q3.c`,
  `q3.err`). (C++ accepts it as a dynamic initialization, P23 above.)
- **Weak constants**: `const int W = 7; #pragma weak W` sets only `is_weak: TRUE` (il_def.h variable bit) and `has_associated_pragma: TRUE` on the variable; the variable's `attributes` is NULL, so no attribute entry
  tells a weak constant from an ordinary one (`/tmp/audit0017/w2.c`, `w2.il`; init_kind is still `initk_static`, constant 7). A weak definition can be replaced at link time, so cerastes must read `is_weak`
  itself and reject it.
- **`sizeof` of an expression differs between the modes**: `const unsigned long SA = sizeof('a');` and `SB = sizeof(1 == 1);` are `unsigned long 4UL` in C23 and `unsigned long 1UL` in C++20 (both
  `{expr=enk_sizeof}`; `p23_sizeof.c|cpp`, `tree.py vars`): the character constant and the comparison have type `int` in C and `char`/`bool` in C++. A named constant with such an initializer has a
  different value per mode, which is the folded value's job (the `enk_sizeof` is encoded as its value, 0010 item 12).

### Silicon facts (spikes S14-S15, `build/m4.2b1/spikes/`, Silicon 6b540aa4, Z3 4.16.0)

- **S14 (`s14a`-`s14e`)**: **Silicon assumes a failed `assert`'s condition and goes on; a path ends only when that assumption contradicts the path.** (Corrected: the first
  version of this entry said "a failed assert ends the path", which `s14d` refutes.) `s14a` (`assert x > 20; assert x > 30; r := 5; assert x > 40`, `ensures r == 0`, `requires 0 <= x <= 10`)
  reports **one error**, the first (`s14a.vpr@6.10--6.16`): `x > 20` contradicts the precondition, so the path is infeasible afterwards; `s14c` (then `assert x > 15`, `assert x < 0`,
  `assert false`) also gives one error. `s14e` (`assert 31 < 5` with a closed false condition, then `assert x > 8` and a failing `ensures`) gives **one error**, at `31 < 5`. `s14d`
  (`assert x > 5` is feasible with the precondition) gives **three errors**: `x > 5`, `x > 8` and `x > 20` (`s14d.vpr@5, @6, @8`). The same failing `assert` text in two methods gives
  **two errors**, one per Viper position (`s14b.vpr@5` and `@12`). So a method stops at a failed obligation whose condition mentions no variable (an initializer or a case value is
  built from constants, so its obligation is closed: its failure means the condition is false) and nothing else is reported after it; an obligation over variables does not end the path.
- **S15 (`s15_edit.py`, `*_p.vpr`)**: the emitted `.vpr` of the four files **already asserts every case value's obligations before the dispatch** (right after the scrutinee temp
  `t__0 := v_x__0`; the dispatch `if (t__0 == <case expr>)` carries none). The hand edit moves them to straight after the parameter copy (`v_x__0 := p_x__0`) and follows each
  case's obligations with `assert <case expr> == <EDG folded>` (`1 * c_pow2(40) == 0`, `(2 + 1) == 3`, `5 == 5`; for the conversion twin `1 * c_pow2(3) == 8`, `2 + 1 == 3`,
  `300 == 44`; for the overflow twin `2147483647 + 1 == -2147483648`, an Int expression that is **false**). Results:

  | file (c23) | Silicon |
  |---|---|
  | `switch_case_expr_shift.c` edited (`shift_p.vpr`) | **1 error**: the shift-count obligation `!(40 < 0 \|\| 31 < 40)` (`shift_p.vpr@17`); the false guard `c_pow2(40) == 0` and the postcondition are not reported |
  | `switch_case_expr_conversion.c` edited (`conv_p.vpr`) | **1 error**: `!(300 < -128 \|\| 127 < 300)` (`conv_p.vpr@23`); the false guard `300 == 44` is not reported |
  | `switch_case_expr_over.c` edited (`over_p.vpr`) | **1 error**: the addition's range obligation (`over_p.vpr@21`); the false guard is not reported |
  | `switch_case_expr.c` edited (`pos_p.vpr`) | **verified** |
  | `pos_bad.vpr` (positive with the guard `2 + 1 == 4`) | 1 error, the guard (`pos_bad.vpr@22`): a guard that is false with passing obligations is reported alone |

  The unedited emission gives the same single errors (`build/cerastes verify ... --std=c23`: `shift`, `conversion`, `overflow` at the case line). So no twin changes its outcome, and a false guard
  after a failed obligation is silent only because the obligation is closed and false (S14e), so the path is infeasible after it: **a guard must come after the obligations of its own case** (and
  after those of every earlier case in the prologue order) or a false guard would be reported first. Not tested: a guard placed before its case's obligations; a prologue shared by methods with several parameters.

## M4.2b2: enumerations

Probes P25-P27 for decision 0018 (enumeration types and enumerators). Evidence is under `build/m4.2b2/probes/` (gitignored), produced with
`scripts/cerastes-fe.sh --c23|--c++20 --display_error_number --il_display <probe>` (output `<probe>.<mode>.il` / `.err`, written by
`build/m4.2b2/fe.sh`) and, for diagnostics, `build/m4.2b2/p27.sh` (plain front end, the front end with verify mode's options from
`src/driver/verify_main.cpp`, and `build/cerastes verify`). The trees quoted are condensed from il_display by the scratch tool
`build/m4.2b2/il.py` (`types`, `enumdef`, `fn`, `cases`, `dik`, `vars`, `ops`, `uses`, `check`; positions omitted; `ENUM:X` marks a constant whose
address is an entry of an enumeration's list, `[named_def]` marks `is_named_constant_definition`; `{expr=...}` is a folded constant's `expr` chain;
`gen` is `compiler_generated`). `il_def.h` lines are in `third_party/edg/src/il_def.h` at the pin. Probe files: `p25_a.c`, `p25_shapes.c|cpp`,
`p25_init.c|cpp`, `p25_use.c|cpp`, `p25_big.c|cpp`, `p25_bool.c|cpp`, `p25_local.c|cpp`, `p25_inc.c|cpp`, `p25_copy.c|cpp`, `p25_chain.cpp`, `p26.c|cpp`
(functions `gN`, `hN`, `sN`, one construct each; in `p26.c` `gN` is on line N+7), `p26_sw.c`, `p26_contract.c|cpp`, `p27_*` (`p27_init_summary.txt` has the
18 initializer cases in both modes). A file that EDG refuses produces no IL, so each refusal has a file of its own. Both modes ran on every probe unless a
construct is invalid in one.

### P25: types, enumerators, how a use is linked to its enumerator: **PASS (rule below), with the copies listed**

**Type entry** (a_type, `kind tk_integer`; `enum_type` :10074, `has_explicit_enum_base` :10069, `is_scoped_enum` :10078, `originally_unnamed` :10112, `bool_type`
:10109, `enum_info` union :10147-10162, `base_type` :9570, `enumerator_list_seen`/`_complete` :9509/:9512, `incomplete` :9808,
`underlying_type_should_use_unsigned` :9525; `tree`: `il.py types`). One entry per enumeration (a C `enum fwd;` before its definition, a C++ opaque
`enum Op : int;` and `enum class SOp;` before theirs give one entry each: `p25_shapes.c|cpp`):

| source | C23 | C++20 |
|---|---|---|
| `enum color {RED, GREEN, BLUE}` | `int_kind unsigned int`, size 4, `constant_list` (RED→GREEN→BLUE via `next`), `enumerator_list_complete` | `int_kind int`, size 4, `underlying_type_should_use_unsigned TRUE` (GNU mode), `constant_list` |
| `enum neg {M = -1, Z, P}` | `int_kind int`, size 4 | `int_kind int`, size 4, no `underlying_type_should_use_unsigned` |
| `enum E8 : unsigned char {A8 = 1, B8 = 255}` | **parsed with no diagnostic**: `has_explicit_enum_base`, `base_type unsigned char`, `int_kind unsigned char`, size 1 | the same |
| `: long` (`BL = 5000000000`), `: unsigned`, `: short` | accepted; `int_kind long` size 8, `unsigned int` 4, `short` 2 | the same |
| `enum EB : bool {FB, TB}` | accepted; `base_type bool`, `bool_type TRUE`, `int_kind unsigned char`, size 1 (`p25_bool.c`) | accepted; `bool_type TRUE`, `int_kind char`, size 1 |
| `enum class S {SA, SB}` / `enum class SB2 : short {X = -3, Y}` | n/a | `is_scoped_enum`, `int_kind int` size 4 / `short` size 2 (`has_explicit_enum_base`, `base_type short`); **`assoc_scope`** (a scope of `kind sck_enum` whose `constants` :18473 chain holds the enumerators); no `constant_list` |
| `enum Empty {};` | n/a (C refuses) | `enumerator_list_complete`, `constant_list NULL`, `int_kind int` |
| `enum { U0, U1 };` | `originally_unnamed TRUE`, no name | the same |
| `typedef enum { T0, T1 } tdef;` | a **`tk_typeref`** entry named `tdef` whose `typeref_type` is the unnamed enum (`p25_shapes.c.c23.il:3144`); a variable of type `tdef` has the typeref as `type` | the enum itself takes the name `tdef` (`originally_unnamed TRUE`) |
| incomplete: `enum inc;` (C), `enum fx : int;` / `enum class SOp;` never defined | `enum inc`: `incomplete TRUE`, size 0, no `enumerator_list_*`; `enum fx : int`: size 4, `has_explicit_enum_base`, `constant_list NULL`, no `enumerator_list_*` | `Op`: the same as `fx`; `SOp`: `assoc_scope NULL`, no flags (`p25_inc.c|cpp`) |

`packed` was never set. Every C++ translation unit also carries the library's `align_val_t` (scoped, `assoc_scope NULL`, `has_explicit_enum_base`, size 8): an encoder
that walks every enumeration type must skip system types.

Enumerators outside `int`, **no diagnostic in either mode** (`p25_big.c|cpp`, `.err` empty): `enum Big {BIG = 3000000000}` is `int_kind unsigned int` (C: enumerator type
`unsigned int`; C++: `Big`), `{B2A = 1, B2B = 4294967295u}` `unsigned int`, `{B3 = -3000000000}` `long` size 8, `{B4 = 2147483647, B4N}` (the implicit
value is 2147483648) `unsigned int`, `{B5 = 4294967296}` `long`/`unsigned long`. So the encoder must reject these itself.

**Type of an enumerator constant** (a_constant `type` :4773, `integer_value` :5041): C, unfixed: **`int`**, a separate type entry (never the plain `int` entry: it has `enum_info.affiliated_type`
:10161 pointing at the enumeration, `p25_a.c.c23.il` `type@7ba8a7fab0` vs plain int `type@7ba8818e00`; for an enum with an enumerator above `INT_MAX` the affiliated entry is
`unsigned int`/`long`); C, fixed (`: unsigned char`...): the enumeration type itself (`A8 type=enum E8 value=1U`); C++ (fixed, unfixed, scoped): the enumeration type
itself (`RED type=color`; `affiliated_type` is NULL in C++, as :10161 says).

**C's compatible type** (`int_kind` of the enumeration, `p25_shapes.c`, `p25_init.c`, `p25_big.c`): no negative enumerator and all fit `unsigned int` → `unsigned int`
(`color`, `fwd`, `U`, `T`, `Big4`); a negative enumerator → `int` (`neg`, `A` of `p25_init.c` with `NEG = -2`) or `long` when below `INT_MIN`; fixed → the base type. C++ unfixed:
`int` whenever the values fit `int`, with `underlying_type_should_use_unsigned` TRUE when none is negative (the `int_kind` is still `int`).

**How a use is linked (the encoder's rule): by flag, and by pointer.**
- The enumerators are `a_constant` entries with **`is_named_constant_definition`** (il_def.h:4931) set; they sit on the enumeration's `constant_list` (C, C++ unscoped) or on
  its `assoc_scope`'s `constants` (scoped). `il.py check`: in every probe the set of constants with the flag equals the set of list/scope members (`p25_*`: `flag-not-in-list=[]
  list-not-flag=[]`, 8, 12, 21, 25, 16, 14, 5 enumerators). No folded constant, cast result or copy carries the flag.
- A reference in code or in a contract is an `enk_constant` expression node whose `constant` **is the list entry** (same pointer): `p25_a.c.c23.il:2550`
  `enum_info.constant_list: file-scope constant@7ba8a7f600: RED`, `:11808` (the node of `c == RED`) `constant: file-scope constant@7ba8a7f600: RED`, and the entry's own block
  (`:2448`) has `is_named_constant_definition: TRUE`. Where the front end folds a use into a larger constant, the enumerator is the innermost `enk_constant` of the `expr` chain, still the
  same pointer: `return RED;` (C) `const int 0 {expr=eok_cast[int](const int 0 ENUM:RED) gen}`; `RED + 1` `const int 1 {expr=add[int](const int 0 {expr=cast[int](const int 0 ENUM:RED) gen}, const int 1)}`;
  `(int)BLUE` `const int 2 {expr=cast[int](const int 2 ENUM:BLUE)}` (non-gen cast); an enumerator in another's initializer (`REF = ADD + 1`) `const int 2 ENUM:ADD {expr=add...}`
  (`p26.c`, `p25_init.c`). In contracts the node is the argument of the walker's `const 0 type=int` and in the IL `eok_ne[int](eok_cast[unsigned int](enk_param_ref) gen,
  eok_cast[unsigned int](const int 0 ENUM:RED) gen)` (`p25_use.c`, `il.py ops`; not folded; positions are the attribute's).
- **Copies.** EDG makes a *fresh* constant of the enumeration's type (value and name equal, no flag, **no `expr`**) in these places only (found by `il.py fn|dik|cases|vars` over
  `p25_copy.c|cpp`, `p25_use.cpp`, `p26.cpp`, `p26_sw.c`):
  1. C++: the initializer of an enum-typed variable or constant from an enumerator of the same enumeration, both unscoped and scoped: a dynamic-init `dik_constant` `color 1` (`color
     d = GREEN;`), `S 0` (`S s = S::SA`), a global `gk: color 1`, `kc`/`ke: color 2` (`p25_copy.cpp`, `p25_local.cpp`);
  2. C++: the case value of `case S::SA:` in a switch over a **scoped** enumeration, `S 0` (`p25_use.cpp.c++20.il:21294` `constant@7b8295b550: SA`, a different pointer from the list entry
     `:4335` `constant@7b82901520`, no flag, no `expr`);
  3. C: the same shapes for a *fixed-base* enumeration: `enum E8 e = A8;` is `dik_constant enum E8 1U`, `enum EU e = U1;` `enum EU 1U`, and `case U0:` in a switch over an
     `enum EU` is `enum EU 0U` (`p25_copy.c`, `p26_sw.c`). For an unfixed C enumeration the same initializers keep the chain
     (`enum color 1U {expr=eok_cast[enum color](const int 1 ENUM:GREEN) gen}`).
  A copy needs no identification: `expr` is NULL, so the encoder reads the constant as a literal of the enumeration type with EDG's value (it equals the enumerator's value). No copy of a
  *non*-enumerator value loses its chain: `constexpr color k7 = static_cast<color>(7); color d = k7;` is `dik_constant color 7 {expr=var k7}` and `k7` is `color 7 {expr=eok_cast[color](const
  int 7)}`; `case ks` (scoped, out of range) `S 77 {expr=var ks}` (`p25_chain.cpp`). So a constant of an enumeration type with neither flag nor `expr` was in every probe an enumerator's copy; the Const range
  check of an enum literal covers it either way.
- **Position and name:** an enumerator carries `source_corresp.name` and `decl_position` (the identifier: `RED` `1:13`, `identifier_range` 13-15, `p25_a.c.c23.il:2448-2470`), usable to report at
  the enumerator's line; `enum_value_range` (a_decl_position_supplement :1975) is the source range of an **explicit** initializer and is absent for an implicit value.

**The enumerator definition constant** (`il.py enumdef`; a_constant `type` :4773, `orig_type` :4777, `expr` :4782; `p25_init.c|cpp`):

| enumerator | C23 | C++20 |
|---|---|---|
| `LIT = 5` | `type int`, value 5, **no `expr`**, `enum_value_range` set | `type A`, value 5, no `expr`, range set |
| `NEG = -2` | `expr` `negate[int](const int 2)` at its own position (L1:25) | the same, `type A` |
| `ADD = 1 + 1` | `expr` `add[int](const int 1, const int 1)` | the same |
| `CASTED = (signed char)300` | value 44, `orig_type int`, `expr` `cast[signed char](const int 300)` | value 44, `type A`, `orig_type int`, the same `expr` |
| `AFTER`, `IMPL` (no initializer) | value previous + 1 (`CASTED` 44 → `AFTER` 45; `REF` 3 → `IMPL` 4; the first is 0), **no `expr`, no `enum_value_range`** | the same |
| `REF = ADD + 1` | `expr` `add[int](const int 2 ENUM:ADD {expr=add...}, const int 1)` | `add[int](const A 2 ENUM:ADD ..., const int 1)` |
| `OV = 2147483647 + 1`, `SH = 1 << 40` | accepted, warnings #61, #63; values -2147483648, 0 (folded, wrapped); `expr` `add[int](...)`, `shiftl[int](const int 1, const int 40)`; the implicit `NOINIT` after `SH` is 1 | **error #28** (not a constant expression; note #2707/#63); no IL |
| `SIZ = sizeof(int)` | `expr` `enk_sizeof[int]`, value 4 | the same |
| `CA = 300u, CB = CA + 1` | `CB` `expr` `add[unsigned int](const int 300 ENUM:CA, const unsigned int 1U {cast...gen})`: the operand is typed `int` (C) / `C` (C++) and carries **no conversion node** under an `unsigned int` operation | `add[unsigned int](const C 300 ENUM:CA, const unsigned int 1U ...)` |
| `DB = (1 > 0)`, `A = 'a'` | `expr` `gt[int](...)` / none (`'a'` literal), value 1 / 97 | `gt[bool](...)` / none |

So: an explicit literal (and `'a'`) leaves `expr` NULL, an implicit value leaves both `expr` and `enum_value_range` NULL: **only `enum_value_range` tells an implicit value from an
explicit literal**, and neither needs an initializer check (an implicit value is previous + 1 by construction; the encoder rejects an enumeration whose values leave `int`). Inside an
enumerator initializer the final type of an earlier enumerator is shown (`const C 300`) while the operation was typed with the definition-time type ([dcl.enum]/5; `300u` → `unsigned int`):
an operand type can differ from the operation's with no cast node, so an initializer must be encoded with the operation's types, the enumerator operand as a literal converted by value.
**In C that typing is wrong** (Opus audit, round 1): C23 6.7.3.3 gives an earlier enumerator type `int` inside the list when its value fits, but EDG's C front end types the reference node the
C++ way (`CA = 300u, CB = (CA - 301) / 2`: the `CA` node is `unsigned int`, the constant `int`; `enum { U = 2u, V = U > -1 };` folds V to 0 where C gives 1; stock `cpfe --c17|--c23` fails
`_Static_assert(CB == 0)`, `decl_spec.c:5930` under `gcc_mode`). So in C the encoder rejects an enumerator reference whose node type differs from the constant's type (decision 0018 item 10).
Also seen: `A = (int)1.5` has `orig_type double` and `expr cast[int](const double ck_float)`; C++ accepts `A = (1, 2)` (`eok_comma`, warning #174), C refuses it (#57); `A = 10 / 0` and `1 << 32` are errors (#39 C,
#28 C++), see P27.

### P26: conversion nodes EDG inserts around enum operands: **PASS (maps below); C leaves some operands bare**

`color c`, `RED`, unfixed non-negative (`p26.c` lines 8-50, `p26.cpp`; `il.py fn gN`). `cast(T)` is a `compiler_generated` `eok_cast`. Outer `cast[int]` in C++ is the
bool-to-int conversion of the `return`.

| construct | C23 | C++20 |
|---|---|---|
| `c == RED`, `c != 1`, `c < GREEN` | `eq[int](cast[unsigned int](c), const unsigned int 0U {cast[unsigned int](const int 0 ENUM:RED)})`: **both operands to the compatible type** (`unsigned int` for `color` and `EU`, `int` for `neg`, `int` for `E8` by promotion) | `cast[int](eq[bool](cast[int](c), const int 0 {cast[int](const color 0 ENUM:RED)}))`: to `int` |
| `c == d` (both enums) | `eq[int](cast[unsigned int](c), cast[unsigned int](d))` | `cast[int](eq[bool](cast[int](c), cast[int](d)))` |
| `c + 1`, `c + RED` | `cast[int](add[unsigned int](cast[unsigned int](c), 1U {...}))`: **unsigned arithmetic** for an unfixed non-negative enum | `add[int](cast[int](c), const int 1)`: **`int`** |
| `RED + 1` | folded `const int 1 {add[int](const int 0 {cast[int](const int 0 ENUM:RED)}, const int 1)}` | the same with `cast[int](const color 0 ENUM:RED)` |
| `-c`, `~c` | `cast[int](negate[enum color](var c))`, `cast[int](complement[enum color](var c))`: the **operation is typed with the enum type, the operand has no cast** | `negate[int](cast[int](c))`, `complement[int](cast[int](c))` |
| `c << 1`, `1 << c` | `cast[int](shiftl[enum color](var c, const int 1))`, `shiftl[int](const int 1, var c)`: left operand and count bare | `shiftl[int](cast[int](c), 1)`, `shiftl[int](1, cast[int](c))` |
| `c && 1`, `!c` | `land[int](var c, const int 1)`, `not[int](var c)`: **bare** | `land[bool](bool_cast[bool](c) gen, ...)`, `not[bool](bool_cast[bool](c))` |
| `if (c)`, `while (c)`, `for (; c;)`, `do ... while (c)`, `c ? 1 : 0` | the condition is `var c:enum color`, **bare** (`stmk_if`, `stmk_while`, `stmk_for`, `stmk_end_test_while`, `eok_question` operand 1; `p26.c` g23, `p26_sw.c` wh/fo/dw/cq) | `eok_bool_cast[bool](var c) gen` |
| `x ? c : RED` | `question[enum color](x, c, const enum color 0U {cast[enum color](const int 0 ENUM:RED) gen})` (type `enum color`) | `question[color](bool_cast(x), c, const color 0 ENUM:RED)` |
| `x ? RED : GREEN` | `question[int](x, const int 0 ENUM:RED, const int 1 ENUM:GREEN)`: **type `int`**, the enumerators bare | `question[color](..., const color 0 ENUM:RED, const color 1 ENUM:GREEN)`: **type `color`** |
| `x ? c : 5` | `cast[int](question[unsigned int](x, cast[unsigned int](c), 5U {...}))` | `question[int](bool_cast(x), cast[int](c), const int 5)` |
| `int i = c` / `unsigned u = c` | `dik_expression` `cast[int](var c) gen` / `cast[unsigned int](var c) gen` (directly, not through the compatible type) | the same |
| `enum color d = c` | `dik_expression var c` (no cast) | the same |
| `enum color d = 2;` | `dik_constant enum color 2U {cast[enum color](const int 2) gen}`, warning #188 | **error #144** |
| `enum color d = (enum color)2;` | `dik_constant enum color 2U {cast[enum color](const int 2)}` (not gen) | `color 2 {cast[color](const int 2)}` |
| `d = RED` | `assign[enum color](var d, const enum color 0U {cast[enum color](const int 0 ENUM:RED) gen})` | `assign[color](var d, const color 0 ENUM:RED)` (no cast) |
| `d = 1` | `assign[enum color](var d, const enum color 1U {cast[enum color](const int 1) gen})`, #188 | **error #513** |
| `ip(c)` (int parameter) | argument `cast[int](var c) gen` | the same |
| `ep(x)` (int to an enum parameter) | argument `cast[enum color](var x) gen`, #188 | **error #167** |
| `return c;` in an `int` function | `cast[int](var c) gen` | the same |
| `return 1;` in an enum function | `const enum color 1U {cast[enum color](const int 1) gen}`, #188 | **error #120** |
| `return BLUE;` in an enum function | `const enum color 2U {cast[enum color](const int 2 ENUM:BLUE) gen}` | `const color 2 ENUM:BLUE` (no cast) |
| `bool b = c` | `dik_expression` `bool_cast[bool](var c) gen` | the same |
| `(enum E8)300` | `const enum E8 44U {cast[enum E8](const int 300)}` (wrapped value 44, **chain kept**, non-gen) | `const E8 44U {cast[E8](const int 300)}` |
| `(enum color)x`, `static_cast<color>(x)` | `cast[enum color](var x)` (non-gen) | `cast[color](var x)` (non-gen; the two spellings are identical) |
| `c++`, `c += 1` | `post_incr[enum color](var c)`, `add_assign[enum color](var c, const unsigned int 1U {...})`: operation typed enum, lhs bare | **error #511** |
| `x += c` | `add_assign[int](var x, cast[unsigned int](var c) gen)` | `add_assign[int](var x, cast[int](var c) gen)` |
| `c \| GREEN` (C++) | not probed in C | accepted by EDG (`p27_bad10.cpp`: no error; `c \|= RED` is #511) |
| `enum E8` (`: unsigned char`): `e + 1`, `e == A8` (C++ also `-e`) | `add[int](cast[int](e), 1)`, `eq[int](cast[int](e), const int 1 {cast[int](const enum E8 1U ENUM:A8)})`: promoted to **`int`** | the same; `negate[int](cast[int](e))` |
| `enum EU` (`: unsigned`): `e + 1`, `e == U1` (C++ also `-e`) | `unsigned int` arithmetic, operands `cast[unsigned int]` | the same; `cast[int](negate[unsigned int](cast[unsigned int](e)))` |
| `enum neg`: `n < Z`, `n + 1` | `int` | `int` |

**Promotion in C++ (STOP rule 2): not triggered.** The unfixed `color` is promoted to `int` in every C++ context (`c + 1` `add[int]`, `RED + 1`, `c + RED`, `~c` `complement[int]`, `x ? RED : 5` `question[int]`),
with `underlying_type_should_use_unsigned` TRUE on the type; `E8` to `int`; only `EU` (fixed `unsigned`) and an out-of-int enumeration give `unsigned int`. **C differs for an unfixed non-negative enumeration:** arithmetic on it
is `unsigned int` (the compatible type), so `c + 1` (and `c - 1`: `RED - 1` would be `UINT_MAX`) differs from C++; with arithmetic on enum operands rejected the comparisons agree (an enum value of the
range checked by the record lies in `[0, 3]`).

**Scoped enumerations** (C++, `p26.cpp` `s1`-`s20`, `p25_use.cpp`): `s == S::SA` is `cast[int](eq[bool](var s, const S 0 ENUM:SA)) gen` (the outer cast is the return's): **no promotion at all**, both operands `S`;
`s < S::SB`, `s <= `, `s > `, `s == t`, `s != t` the same; `static_cast<int>(s)` `cast[int](var s)` (non-gen), `static_cast<S>(x)` `cast[S](var x)`, `static_cast<bool>(s)` `cast[int](bool_cast[bool](var s))`;
`return S::SB` `const S 1 ENUM:SB`; `s = S::SB` `assign[S](var s, const S 1 ENUM:SB)`; `s ? ...` is refused by the language. Contracts: `r == S::SB` `eq[bool](enk_param_ref[S], const S 1 ENUM:SB)` (`p26_contract.cpp`).

**`switch`** (`il.py fn`/`cases`; `p26_sw.c`, `p26.cpp`, `p25_use.*`; the case entry's `case_value` is a_switch_case_entry :16259):

| subject | scrutinee | case value (`case X:`) |
|---|---|---|
| C, unfixed `color` | `var e:enum color`, **bare** | `enum color 0U {cast[enum color](const int 0 ENUM:RED) gen}` |
| C, `neg` | `var e:enum neg`, bare | `enum neg -1 {cast[enum neg](const int -1 ENUM:M {negate...}) gen}` |
| C, `EU` (`: unsigned`) | `var e:enum EU`, bare | `enum EU 0U`, **a copy (no chain)** |
| C, `E8` (`: unsigned char`) | `cast[int](var e) gen` | `int 1 {cast[int](const enum E8 1U ENUM:A8) gen}` |
| C, `int x` with `case RED` | `var x:int` | `int 0 {cast[int](const int 0 ENUM:RED) gen}` |
| C, `case 1` in a switch over `color` | bare | `enum color 1U {cast[enum color](const int 1) gen}`, warning #188 |
| C++ `color`, `E8`, `neg` | `cast[int](var e) gen` | `int 0 {cast[int](const color 0 ENUM:RED) gen}` |
| C++ `EU` | `cast[unsigned int](var e) gen` | `unsigned int 0U {cast[unsigned int](const EU 0U ENUM:U0) gen}` |
| C++ scoped `S` | `var s:S`, bare | `case S::SA` `S 0`, **a copy**; `case static_cast<S>(1)` `S 1 {cast[S](const int 1)}`; `case ks` (a `constexpr S`) `S 77 {expr=var ks}` |

**Where C leaves an enum operand without a conversion node** (record-amendment item, not a stop): the switch scrutinee of an enumeration whose compatible type is `int` or wider (`color`, `neg`, `EU`; `E8` is promoted);
every condition (`if`, `while`, `for`, `do`, `?:`, `&&`, `||`, `!`); the operand of unary `-` and `~` and both operands of `<<` (the operation itself is typed with the enum type, its result then cast to `int`); the
left operand of `++`, `--` and compound assignment; the arms of a `?:` whose arms are both of the enum type (the `?:` is then typed `enum color`). Converted: both operands of `==`, `!=`, `<`..., of `+`, `-`, ..., the
right operand of compound assignment, arguments, `return`, assignment to an integer.

### P27: diagnostics: **PASS** (`p27.sh`, `p27_*`; plain front end, the front end with verify mode's flags, and `build/cerastes verify`)

Verify mode's EDG options were, when probed, `--diag_suppress=61,62,63,39,179,68,69,546,940,1305,2803`, `--diag_error=1901` (C only) and `--diag_error=161,1675` (`verify_main.cpp:324-332`), and
`build/cerastes verify` stopped at the declaration (`unsupported_declaration: an enumeration`). *Since M4.2b2 (decision 0018) verify mode also suppresses #188 (below), and enumerations are encoded.*
- **Casts out of range, no diagnostic in any mode or option set:** `(enum E8)300` (value 44), `(enum color)7`, `(enum color)-1` (C: `4294967295U`; C++: `-1`), `(enum neg)5`, `static_cast<color>(7)`, `static_cast<S>(77)`, `static_cast<S8>(300)`;
  the chain `{expr=eok_cast[...](const int 300)}` is kept (`p27_cast.c|cpp`, `il.py fn f1..`), so the encoder can check them (the folded value for a fixed type is wrapped).
- **An enumerator out of `int`**, `enum Big {BIG = 3000000000}` and the other four shapes of P25: no diagnostic (`p25_big.*.err` empty).
- **Mixed enum/int, C23** (`p27_mixed.c`): **warning #188-D** "enumerated type mixed with another type" (not suppressed by verify mode when probed; suppressed since 0018, every such conversion being checked) at `enum color d = 2;`, `d = 1;`, `ep(x)` (int to an enum
  parameter), `return 1;`, `return o;` (another enumeration) and `case 1:` / `case O1:` in a switch over `color`; **no diagnostic** for `c == 1`, `c == o` (two enumerations), `c++`, `c += 1`, `x += c`, `c < 1.5`, `ip(c)`.
- **C++ refuses what C warns about** (`p27_bad1-9.cpp`, plain mode): `color d = 2` #144, `d = 1` #513, `ep(x)` #167, `return 1` #120, `return o` (other enumeration) #120, `c++`, `++c`, `c += 1`, `c |= RED` all #511 ("requires an applicable
  user-defined operator function"); `c | GREEN` and `c & GREEN` are accepted (`p27_bad10|11.cpp`).
- **A switch that misses an enumerator and has no default**: no diagnostic in either mode (`m12` in `p27_mixed.c|cpp`).
- **Enumerator initializers** (`p27_init_summary.txt`): `const int K = 1; enum { A = K };` C: warning #1901 plain, **error #1901 with verify's `--diag_error=1901`** (also `static const`); C++: accepted, `expr` is `var K`. `A = 1.5` and `1.0`: errors in both
  (#157 C, #2412 C++). `A = (int)1.5`: accepted in both (`expr cast[int](const double ck_float)`, `orig_type double`). `A = (1, 2)`: C error #57; C++ warning #174, `eok_comma`. `A = 2147483648`, `{A = 2147483647, B}`, `1 << 31`: silent. `A = 2147483647 + 1`,
  `1 << 32`: C warnings #61, #63 (suppressed by verify mode; the values wrap), C++ **error #28**. `A = 10 / 0`: C error #39, C++ error #28.
- **Incidental, predates this session** (`p27_init14.cpp`, `p27_init16.cpp`, `p27_gen1.cpp`): in C++, an error #28 that carries a *note* that verify mode suppresses (`#39` division by zero, `#63` shift count: `A = 10 / 0`, `A = 1 << 32`, `constexpr int z = 1 << 32;`) makes EDG fail with
  `internal error #2656: assertion failed: determine_severity_code: bad severity`, and `build/cerastes verify` ends with exit 3 `internal error in the front end (the program was not checked)`. It is a tool failure, never a verdict (`p27_gen2.cpp`, `case 10 / 0`, is an ordinary
  error #39). Not specific to enumerations.

## M4.2c: locals without an initializer, #2656, -Wswitch

Probes P28-P31 for decision 0019 (indeterminate values; P30 and P31 below) and spike S16. Evidence is under `build/m4.2c/probes/` and `build/m4.2c/spikes/` (gitignored), produced with
`scripts/cerastes-fe.sh --c23|--c++20 --display_error_number --il_display <probe>` (`build/m4.2c/fe.sh`: output `<probe>.<mode>.il` / `.err`), the scratch tools `il.py` (`fn`,
`types`) and `tree2.py dik` (copies of the M4.2b2 tools; `il.py fn` now also follows `body_statement`, `initialization`, `increment` and `loop_statement`), `build/m4.2c/p29.sh` (the front
end with verify mode's option list, `src/driver/verify_main.cpp:299-334` at this commit, plus extra options) and `build/cerastes verify` (`*.verify*.txt`). Silicon was run as
`scripts/run_tests.py` runs `.vpr` fixtures (`build/m4.2c/spikes/silicon.sh`: `java -Xss128m -jar silicon.jar --z3Exe ... --numberOfErrorsToReport 0`). `il_def.h` lines are in
`third_party/edg/src/il_def.h` at the pin. Probe files: `p28.c` (functions `g1`-`g13` on lines 2-14), `p28x.cpp` (the same without `g6` and `g8`, `bool` for `_Bool`),
`p28_init.cpp|p28_init2.cpp`, `p28_self.c`, `p28_reg.c`, `p29.c` (functions `d1`-`d18`, one per line), `p29_jump.c` (`j1`-`j8`, split into `j1.c|cpp`...), `p29_acc.c`.
A `.cpp` file is compiled in the mode of the option, not of its extension, so each probe ran in both modes unless a construct is invalid in one.

### P28: IL of a local without an initializer: **PASS (the stop rule did not trigger)**

Every declaration without an initializer, in both modes, has `init_kind initk_none` (`an_init_kind` :11014, `a_variable.init_kind` :11088) and **no `stmk_init` statement**; the
declaration is a `stmk_decl` whose `entities` list (:16802, a list: several variables per statement) holds it. `tree2.py dik` (`probes/p28.c.c23.il`, `probes/p28x.cpp.c++20.il`):

| source | function | variable | C23 | C++20 |
|---|---|---|---|---|
| `int x;` | `g1` | `x` | `type=int sc=sc_auto init_kind=initk_none` | the same |
| `int x, y = 1;` | `g2` | `x` / `y` | `initk_none` / `initk_dynamic` (one `stmk_decl`, then one `stmk_init` with `dik_constant 1`) | the same |
| `int y = 1, x;` | `g3` | `y` / `x` | `initk_dynamic` / `initk_none` | the same |
| `_Bool b;` / `bool b;` | `g4` | `b` | `type=bool ... initk_none` | the same |
| `enum color c;` (file-scope `enum color {RED, GREEN, BLUE}`) | `g5` | `c` | `type=enum color ... initk_none` | `type=color ... initk_none` |
| `const int kk;` | `g6` | `kk` | `type=const int ... initk_none` (**front end: warning #257-D, below**) | **refused: `error #257: const variable "kk" requires an initializer`** (`p28.c.c++20.err`) |
| `for (int i; ; ) {...}` | `g7` | `i` | `initk_none`; the `stmk_decl` is the `for`'s `initialization:` (`il.py fn g7`) | the same (C wraps the `for` in a compiler-generated block; C++ does not) |
| `register int r;` | `g8` | `r` | `sc=sc_register init_kind=initk_none` (il_def.h `sc_register` :424) | **refused: `error #2798: use of the "register" storage class specifier is not allowed`** |
| `unsigned char u;` | `g9` | `u` | `type=unsigned char ... initk_none` | the same |
| `int t;` in a `while` body | `g10` | `t` | `initk_none`; the `stmk_decl` is the first statement of the loop body (`loop_statement:`) | the same |
| `int x; if (n) goto L; x = 1; int y; y = 2; L: return n;` | `g11` | `x`, `y` | both `initk_none`, two `stmk_decl`s, the `stmk_label` after the second (inside the function block) | the same |
| `switch (k) { int x; case 1: ... }` | `g12` | `x` | `initk_none`; the `stmk_decl` is the **first statement of the switch's `body_statement` block, before the first `stmk_switch_case`** | the same |
| `switch (k) { case 1: int x; x = 2; ... case 2: ... }` | `g13` | `x` | `initk_none`; the `stmk_decl` follows the first `stmk_switch_case` | the same |

Statement trees (`il.py fn`, condensed; `probes/p28.c.c23.trees`, `probes/p28x.cpp.c++20.trees`; only the C blocks marked `gen` differ from C++, where `if`/`while`/`switch`/`for`
stand directly in the function block and a condition is wrapped by `eok_bool_cast [gen]`):

```
g1:  stmk_block { stmk_decl; stmk_expr eok_assign[int](var x, var k); stmk_return var x }
g3:  stmk_block { stmk_decl; stmk_init; stmk_expr eok_assign[int](var x, var k); stmk_return eok_add(var x, var y) }
g12: stmk_block { stmk_block gen { stmk_switch var k  body_statement: stmk_block { stmk_decl; stmk_switch_case; stmk_expr eok_assign(var x, const int 2); stmk_return var x; stmk_switch_case; stmk_return const int 0 } } }
g13: ... body_statement: stmk_block { stmk_switch_case; stmk_decl; stmk_expr eok_assign(var x, const int 2); stmk_return var x; stmk_switch_case; stmk_return const int 3; stmk_switch_case; stmk_return const int 0 }
```

- **One `stmk_decl` can declare several variables** (`entities` of `g3`'s declaration is `variable@...bd190` (`y`) followed by `variable@...bd570` (`x`), `probes/p28.c.c23.il:13301-13302`), and the **`stmk_init` of
  `y` comes after the whole `stmk_decl`**, i.e. after `x`'s declaration: `int x, y = x;` reads `x` in `y`'s initializer, after `x`'s declaration point (`p28_self.c` line 2).
- **The assignment of a no-initializer local is an ordinary `stmk_expr` `eok_assign`** (`g1`, `g4`, `g5`, `g9`: the right side carries the usual conversions, e.g. `eok_bool_cast`, an `eok_cast` of an enumerator in C, a folded constant with `expr`).
- **Today's encoder** rejects each of these at `src/backend/encode.cpp:1566` (`v->init_kind == initk_none`: `unsupported_declaration` "a local variable without an initializer", `probes/p28.c.verify.c23.txt`).
  In a switch body it *also* rejects what surrounds the declaration, whether or not it has an initializer: `encode.cpp:2224` "a statement before the first case label of a switch (it is unreachable)" (`g12`: the
  declaration before `case 1:`), and `encode.cpp:2165` "a case label that jumps over the declaration of x" (`g13`: `case 2:` after the declaration; `probes/p28.c.verify.c23.txt`, lines 13 and 14). The VIR checker holds the same two rules
  (`src/vir/check.cpp:375-400`: `first` and `declaration_seen`) and a third for `goto` (`check.cpp:745-752`, `GotoProblem::Kind::skips_declaration`).
- **C++ initializers that are not `initk_none`** (`probes/p28_init.cpp.c++20.il`, `tree2.py dik`):

  | source | `init_kind` | statement | `build/cerastes verify` today (`probes/p28_init.cpp.verify.txt`, `p28_init2.cpp.verify.txt`) |
  |---|---|---|---|
  | `int x{};` (`h1`), `int x = {};` (`h2`), `int x = 0;` (`h6`) | `initk_dynamic` | `stmk_init`, `dik_constant 0` | accepted |
  | `int x = int();` (`h3`) | `initk_dynamic` | `stmk_init`: a `dik_zero` entry, then `dik_constant 0 {expr=enk_temp_init}` | **rejected: `unsupported_expression` "an expression of kind enk_temp_init"** (not a no-initializer local; unchanged by M4.2c) |
  | `int x{k};` (`h4`), `int x(k);` (`h7`) | `initk_dynamic` | `stmk_init`, `dik_expression` | accepted |
  | `bool b{};` (`h8`) | `initk_dynamic` | `stmk_init`, `dik_constant [bool 0]` | accepted |
  | `int x();` (`h5`) | (a function declaration: no variable) | `stmk_decl` of a routine | rejected: `unsupported_declaration` (unchanged) |

  So value-initialization is already an initializer with a constant 0 and needs nothing new. `h3` and `h5` were removed to get `p28_init2.cpp` (6 functions) to `verified`.
- **A local used in its own initializer stays refused** (`p28_self.c` line 1, `int x = x + n;`: `unsupported_declaration` "a local variable used in its own initializer"); `int x, y = x;` (line 2) is
  refused today only because `x` has no initializer, and becomes an `uninitialized` read of `x` (its flag is false) with the design.
- `register int r = 1;` and `auto int a = 1;` are accepted today in C23 (`p28_reg.c`), so only the missing initializer stands in the way of `g8`.
- `--cerastes_walk` prints routines, signatures and attributes, not declarations or initializers, so it adds nothing for these shapes; `il_display` and the trees above are the evidence.

### P29: front-end diagnostics: **PASS (findings below)**

`p29.sh` runs `cerastes-fe` with verify mode's current list (`--display_error_number --no_wrap_diagnostics --no_colors --diag_suppress=61,62,63,39,179,68,69,188,546,940,1305,2803`, `--diag_error=1901` in C,
`--diag_error=161,1675`) plus extra options; the outputs are `probes/p29.c.c23.verifyflags.txt`, `p29.c.cxx.verifyflags.txt`, `p29_jump.c.*.verifyflags.txt`, `jump_plain.txt`.

**Used before set, #549** (`warning #549-D: variable "x" is used before its value is set`, discretionary; `p29.c`, **identical in C23 and C++20**):

| line | source | #549 |
|---|---|---|
| 1 | `int x; return x + n;` | **yes** |
| 2 | `int x; if (c) x = 1; if (c) return x; return 0;` (path-precise correct) | no |
| 3 | `int x; if (c) x = 1; return x;` (read possibly uninitialized) | **no** (a textual set before the read silences it) |
| 4, 5 | `int x; x++;`, `int x; x += 1;` | **yes** (a compound assignment reads) |
| 8, 9 | `x = 1; return x + n;`, one assignment per `if`/`else` arm | no |
| 10 | `int x; while (n > 0) { if (n == 3) x = 1; else return x; n--; } ...` | no (outer variable, uncompleted loop: `symbol_ref.c:2633-2650`) |
| 11 | `int x; for (;;) { if (n) return x; x = 1; }` | no (the same rule) |
| 12 | `int x; if (x) return 1; x = n; ...` | **yes** |
| 13 | `int x; x = x + 1;` | **yes** |
| 14 | `int x, y = 1; y = n; return x;` | **yes** |
| 18 | `int x; int y; y = x; x = 1; return y + n;` | **yes** (a use before the textual set) |
| `p29_acc.c:3` | `int x; if (0) return x; x = n; return x;` (correct: the read is dead) | **yes** (a spurious warning on a program cerastes accepts) |
| `p29_acc.c:4` | `int x; return n + (int)sizeof(x);` | no |
| `p28_self.c:1,2` | `int x = x + n;`, `int x, y = x;` | yes, yes |

EDG decides this by the order of the source, not by paths: it warns only at the **first use** of a variable (`vp->used` is then set, `symbol_ref.c:2555-2557`) that has no textual set before it
(`sym_ptr->value_has_been_set`), and stays silent when the use is inside a loop or after a label that "may be reached by a branch back" (`symbol_ref.c:2633-2650`: `last_label_decl_seq`, `is_loop_scope`). So #549 neither
finds every read that cerastes will reject (line 3: one-armed `if`) nor spares every read cerastes accepts (`p29_acc.c:3`); **it is not evidence for or against an `uninitialized` verdict.**

**Other warnings the shapes print** (same files): `#550-D variable "x" was set but never used` (`d6`, `int x; x = 1; return n;`; `d14` for `y`; `j1`), `#177-D variable "x" was declared but never referenced`
(`d7`, `int x; return n;`; `j2`), `#185-D dynamic initialization in unreachable code` (`j8`), and, for C only, **`#257-D const variable "kk" requires an initializer`** (`g6`; C++ has the hard error #257).
None of #550, #177, #257 is in verify mode's suppression list today; all are printed on stderr by `build/cerastes verify` (`probes/p28.c.verify.c23.txt` starts with the C23 `#257`, `#549`, `#550`).

**#546** (`transfer of control bypasses initialization of`), `p29_jump.c`, split files `j1`-`j8`, plain front end (`jump_plain.txt`) and verify mode's list (`p29_jump.c.*.verifyflags.txt`):

| source | C23 | C++20 |
|---|---|---|
| `j1` `if (n) goto L; int y; y = 2; L: ...` (goto past a declaration **without** an initializer) | no #546 | no #546 |
| `j3`, `j4` (the same with the label before/after the assignment) | none | none |
| `j5` `switch (k) { ... case 1: int x; x = 2; ... case 2: ...}` (case past a declaration without an initializer) | none | none |
| `j7` `switch (k) { int x; case 1: ... default: x = 0; ...}` | none | none |
| `j2` `if (n) goto L; int y = 2; L: ...` (goto past a declaration **with** an initializer) | `warning #546-D` | **`error #546`** (not discretionary: `--diag_suppress=546` does not hide it; `p29_jump.c.cxx.verifyflags.txt`: `error #546` and `1 error detected`) |
| `j6` `case 1: int x = 5; ... case 2:`, `j8` `switch (k) { int x = 7; case 1: ... }` | `warning #546-D` | `warning #546-D` (suppressed by verify mode) |

So **no #546 appears for a jump past a declaration without an initializer** (as expected), and the C++ goto past an initialized declaration is a front-end error that cerastes did not control and does not change.

**Suppressing #549** (`p29.sh p29.c c23 --diag_suppress=549` and `... --no_use_before_set_warnings`, `probes/p29.c.c23.sup549.txt`, `p29.c.c23.nubs.txt`): the two outputs are **identical**: every #549 is gone, and
#550 (lines 6, 14) and #177 (line 7) remain. A second `--diag_suppress=` option accumulates with the first (`p29_acc.c` line 2: #188 stays suppressed). `--no_use_before_set_warnings` (`cmd_line.c:393`, short `-j`, sets
`suppress_used_before_set_warnings`, `cmd_line.c:10835-10839`) skips the whole check at `symbol_ref.c:2556`; `vp->used = TRUE` (what #177 and #550 read) is still set, so no other warning changes.

**A diagnostic pragma overrides the number but not the option** (`p29_prag.c`: `#pragma diag_error 549` before `int p1(int n) { int x; return x + n; }`): with `--diag_suppress=549` the front end prints `error #549-D: variable "x" is used before
its value is set` (`p29.sh p29_prag.c c23 --diag_suppress=549`, and `build/cerastes verify` the same, `1 error detected`); with `--no_use_before_set_warnings` it prints nothing (the check is not made). Verify mode rejects a diagnostic pragma in a verified file
(`diagnostic_pragma_in_verify`), but one in a system header would still apply.

**Recommendation: add `--no_use_before_set_warnings` to the EDG command line** (`verify_main.cpp`, next to the suppression list, with a comment that cerastes reports the read itself as `uninitialized`, 0019). Both forms give identical output on every probe, and #549 must go: it is a
*warning on stderr*, and a test whose stderr is not empty fails (`tests/README.md`: "a front-end warning in a test is unexpected output: tests are warning-free"), so every `uninitialized` must-fail test, and every correct program with a dead-code read, would fail on EDG's
own #549 line. The option is chosen over the number because it cannot be turned back on by a pragma (above); the number form would turn a pragma in a system header into a spurious rejection (fail closed, but a wrong one). Open question for the owner: #550 and
#177 also print for natural `uninitialized` tests (`int x; return n;`, `int x; x = 1; return n;`), so test authors must write variables that are read; whether verify mode should suppress them too is a separate decision (it changes only stderr).

### Silicon facts (spike S16, `build/m4.2c/spikes/`, Silicon 6b540aa4, Z3 4.16.0)

Hand-written Viper in the style `lower.cpp` emits (method locals declared at the top, `v_x` the variable, `d_x` the flag, `ds_x__N` the snapshot before loop `N`, `while`/`goto`/`label`, the `goto l_end; assert false; label l_end` ending;
no inhale and no assume; `scripts/lint_viper.py` accepts every construct, its only complaints being the hand-written `f_abort__9` without its trusted tag and the missing `.map`). The flag is reset by `d_x := false; v_x := 0`.

| file | result |
|---|---|
| `s16_ok.vpr` (11 methods and the trusted `f_abort__9`) | **verified** |
| `s16_more_ok.vpr` (3 methods) | **verified** |
| `s16_fail.vpr` (8 methods) | 8 flag errors, each exactly at the marked `assert d_*` line, plus 2 range errors in `m_goto_noentry` (below) |
| `s16_more_fail.vpr` (3 methods) | 3 errors, each at its marked line |
| `s16_g.vpr` | 1 error, the control (below) |
| `s16_cascade_{0,1,min}.vpr` (8 methods each), `s16_cascade_loop.vpr` | see (f) |

- **Must verify** (`s16_ok.out`, `s16_more_ok.out`: "Silicon finished verification successfully"):
  (a) `m_a` assign then read; `m_ifelse` both arms assign; `m_pathprecise` `if (c) x = 1; if (c) return x;`; `m_abort` one arm calls a bodyless method `ensures false`, the other assigns, the read follows;
  (b) `m_loop_outer` (x assigned before the loop, read and reassigned in it; invariants in `lower.cpp`'s order: the automatic range invariant, `ds_x__1 ==> d_x`, the user invariants, among them `invariant d_x`, the flag obligation of a user invariant that reads `x`);
  (c) `m_forever_break` (`while (true)`, assignment, `goto l_brk`, read after); `m_do` (`while (true)` with the exit test at the end, `x = i` before the test, read after the loop); `m_continue` (a `continue` skips the assignment, the read is after it in the same iteration);
  (e) `m_switch` (the reset before the dispatch, each case assigns before it reads), `m_switch_fall` (the label reached by fallthrough reads nothing of `x`);
  `m_goto_ok` (the block-entry reset, a forward `goto` past the declaration, nothing read after the label); `m_body_local` (a body-local flag reset in the body, **no invariant mentions it**: (g) second half);
  `m_nested` (`x` outer to two loops: one snapshot per loop, read after the inner loop and after the outer one).
- **Must fail, at the read, with Silicon's own text** (`s16_fail.out`, `s16_more_fail.out`). A flag read is `Assert might fail. Assertion d_x might not hold.` (silver `assert.failed:assertion.false`), at the `assert d_*` line:

  | | method | line | error |
  |---|---|---|---|
  | (a') | `m_a1` (one-armed `if`) | 17 | `Assert might fail. Assertion d_x might not hold. (s16_fail.vpr@17.10--17.13)` |
  | (b') | `m_loop_nosnap` (the loop of `m_loop_outer` without `ds_x__1 ==> d_x`) | 45 | the same at the read in the body: **the snapshot invariant is needed** |
  | (b'') | `m_loop_after` (read after a loop that may not run) | 82 | the same |
  | (h2) | `m_loop_read_first` (read before the assignment in the body, `x` outer, never assigned before) | 111 | the same |
  | (d) | `m_goto_read` (block-entry reset, goto past the declaration reset, read at the label) | 141 | `Assert might fail. Assertion d_y might not hold.` **and nothing else** |
  | (d2) | `m_goto_noentry` (the same **without** the block-entry reset) | 164 | the flag error **and two postcondition errors** (`Postcondition of m_goto_noentry might not hold. Assertion -2147483648 <= res / res <= 2147483647 might not hold`): on the goto path `v_y` is an unconstrained `Int`, so the **dummy value is needed on that path too**, not only the flag |
  | (e') | `m_switch_read` (the case reads `x`; the reset is before the dispatch) | 186 | the flag error |
  | (h) | `m_body_local_read` (body-local `t`, read in iteration 2 before `t = i`) | 218 | the flag error (the declaration reset makes it fail in every iteration that reaches the read) |
  | | `m_nested_read` (inner loop may not run), `m_switch_fall_read` (dispatch to `case 2` skips `x = 1`) | 41, 72 | the flag error |
  | | `m_inv_entry` (`invariant d_x`: a user invariant that reads `x` before any assignment) | 102 | **`Loop invariant d_x might not hold on entry. Assertion d_x might not hold.`** (silver `invariant.not.established`): the flag obligation of an invariant is reported as an invariant check, like every obligation clause (`obligation_clause`) |

- **(f) The dummy value does not matter for a cascade; the knowledge of the flag does** (`s16_cascade_{0,1,min}.out`, gen by `gen_cascade.py`; `int x; return <e>;` with `e` in `100 / x`, `100 % x`, `100 / (x - 1)`, `x - 1`, `x + 1`, `-x`, `x * 2`, `INT_MAX / x`, each with the
  obligations `lower.cpp` emits, `v_x := d` for `d` = 0, 1, `INT_MIN`): **every one of the 24 methods reports exactly one error, the flag assert**, for all three dummies, including `100 / x` with 0 and `x - 1`, `-x` with `INT_MIN`. This is S14's rule
  (a failed `assert` is assumed and the path goes on, ending only if the assumption contradicts it): the flag was just set to `false` on this very path, so assuming `d_x` makes the path infeasible and no later obligation is reported. The
  cascade does appear when **the flag is not known on the path**: `s16_cascade_loop.vpr` (`x` outer to a loop that assigns it; `100 / x` read first in the body, dummy 0) reports **two errors**, `assert d_x` (31) and `assert v_x != 0` (32): after the loop head's havoc the
  flag is unknown, so assuming it does not end the path and `x` is any in-range value, whatever the dummy. (A genuine report: `x` could also be 0 there.) **Consequences:** (1) the dummy only has to be a value of the type's range (the automatic range invariant, `lower.cpp:686-721`,
  must hold when the flag is false, (d2) above), so `0` for `int`/`unsigned`/`char`s, `false` for `bool`, and for an enumeration `0` **only if 0 is in its value range** (decision 0018's `[lo, hi]`): otherwise `lo`; a literal in range avoids any new obligation; (2) a second
  error at the same line after an unknown-flag read is possible and is not a bug of the dummy; (3) after the first uninitialized read on a path with a known flag, later errors of that path are not reported (S14).
- **(g)** `s16_g.vpr`: after a loop that does not assign the snapshot, `assert ds_x__1` verifies with **no invariant naming the snapshot**, i.e. Viper does not havoc a variable the loop does not assign; the control `assert d_x` (the loop assigns the flag) fails (`s16_g.vpr@31`), so the flag itself is havocked at the head
  (which is why the snapshot invariant is needed). A flag reset inside a loop body needs no invariant (`m_body_local`, `m_body_local_read`).
- **Order of the three resets matters only for what the flag says at a label**: the block-entry reset is what makes `(d)` exact (one error, no range errors) and `(d2)` shows what is lost without it.

### P30: the front end's internal error #2656 on a suppressed note: **PASS** (cause found; fixed on fork branch `0022`, not pinned)

Evidence: `build/m4.2c/edg2656/` (`matrix.txt`, `compare.txt`, `out/<probe>.{A,B,C}.txt`, `drv_*.txt`, `repro-*.txt`,
`edgy-*.txt`). Probes are `edg2656/probes/*` (126 files: C++ `constexpr` variable, enumerator, `static_assert`, array
bound, template argument, `case`, `if constexpr`, `alignas`, bit-field width, `consteval`, `constexpr` call chains,
lambdas; C enumerator, `_Static_assert`, case, file-scope initializer, array bound, bit-field; run-time uses), run
with the front end and verify mode's options (`edg2656/fe.sh`).

- **Cause** (`third_party/edg/src/error.c`):
  - `create_diagnostic_entry` (:5032) applies `check_for_overridden_severity` to every primary-kind entry, including
    the `es_more_info` notes that the constant-expression interpreter attaches to error #28 (`add_more_info_list`,
    :7678).
  - `es_more_info` is below `es_discretionary_error`, so a suppressed number gives the note `es_none`.
  - Displaying the error then reaches `determine_severity_code` (:3023), which asserts:
    `internal error #2656: assertion failed: determine_severity_code: bad severity`.
- **Which suppressed numbers are notes** (each suppressed alone):
  - #39 (division and remainder by zero), #62 (shift count negative) and #63 (shift count too large);
  - #61 (integer operation result out of range), only for a shift count or pointer offset that overflows the host
    integer (`1 << (unsigned __int128)-1`);
  - #68, #69, #179, #188, #546, #940, #1305 and #2803 are never notes (outside `interpret.c` they are primary
    diagnostics only).

  An ordinary overflow in a constant expression gives note #2707 (and #2706), which verify mode does not suppress.
- **Where it fails.**
  - **C++:** 49 of the probes, exit 4 from the front end and exit 3 from `cerastes verify`. Every C++
    constant-expression context fails for a #39, #62 or #63 note, except a C++ `case` label (an ordinary error #39)
    and a narrowing template argument (#2361).
  - **C:** never, because C has no interpreter notes. Run-time uses (`x << 40`, `x / 0`, `x << -1`,
    `2147483647 + 1 + x`) print nothing with verify mode's options and warnings #63, #39, #62 and #61 without them,
    so the suppression does real work.
- **Remarks instead of suppression** (`compare.txt`). Making #39, #61, #62 and #63 remarks (`--diag_remark`), or all
  twelve numbers, gives byte-identical output on all 126 probes:
  - the 49 failures become an error #28 with exit 2;
  - run-time uses still print nothing;
  - every other probe is unchanged.

  **But the front end then prints the error twice** under a "1 error detected" summary: the overridden note is no
  longer `es_more_info`, so `construct_text_message` writes it as a diagnostic of its own and then again within its
  error (`out/cxx_enumerator_div.cpp.B.txt`). The test harness rejects that (a summary that does not match the errors
  printed), so verify mode keeps its suppression list and waits for the fix (owner, 2026-10-22; decision 0019 item 9).
- **Upstream.** Reproduced with stock `cpfe` on upstream `main` `ec2f12cffa` (`repro-before.txt`). Fixed on fork
  branch `0022` (`3b05dd5c43`): `add_more_info_list` frees a note whose severity is `es_none`. The `GH-*` test fails
  before and passes after, natively (`edgy-before-vs-recording.txt`, `edgy-after.txt`). The bug file is
  `internal/edg_bugs/0022-suppressed-note-internal-error.md`.
- **Incidental, not pursued:** `constexpr int z = (-2147483647 - 1) / -1;` (and `% -1`) gives no diagnostic at all in
  this front end (probes `cxx_note_1`, `cxx_note_2`). cerastes encodes initializers as written (0017), so it does not
  rely on the fold.

### P31: `-Wswitch` on switches over an enumeration: **PASS (matrix below; the compilers disagree on 6 of 71 shapes)**

The test-mutator's matrix (`build/m4.2c/tests-prep/matrix.txt`, `matrix/results.json`, one file per shape in `matrix/*.c|*.cpp`, generator `gen_matrix.py`) compiles 71 shapes, each in C and in C++ where
valid, with Apple clang (Command Line Tools) and GCC 16 (Homebrew `gcc-16`, `g++-16`), `-std=c23` and `-std=c++20`, `-Wall -Wextra -Werror -U__CERASTES__`. A cell is `ok` or a diagnostic; `show_matrix.py`
prints the diagnostics, `build/m4.2c/summ_matrix.py` classifies them. Of the 71 shapes (a shape that C++ only, `enum class`, is `n/a` for C): **25 are accepted by every compiler and mode, 40 are warned by every
compiler and mode that can compile them, 6 differ.**

**What every compiler and mode warns on** (the two `-Wswitch` rules, `-Werror,-Wswitch` in clang, `-Werror=switch` in GCC):

- **Rule 1, a case that is not an enumerator's value.** `case 4:` over `enum color { RED, GREEN, BLUE }` (`unfixed_case4_default`, `_covering`); `case 5:` over `enum neg { -1..1 }`; `case -2:`; over fixed bases, `case 2:` over
  `enum uc : unsigned char { U0, U1 }`, `case 200:`, `case 5:` over `: unsigned long`, `case 7:` over `: int`; a value that does not fit the base also gets `-Wswitch-outside-range` in GCC (`case 300:` over `: unsigned char`)
  and an overflow note in clang C, and `case -1:` over `: unsigned long` gets a narrowing warning in C++ besides. A cast out of range, `case (enum color)7:`, warns in all but clang C++20, which refuses it as not a constant expression. `enum class` too
  (`class_cast_case7_default`, `class_uchar_case_cast200_default`). A `case` named by an out-of-range integer constant object (`const int K4 = 4;`, C++) warns too.
- **Rule 2, an enumerator without a case and no `default`.** One missing enumerator (`unfixed_missing_blue_nodefault`, `fixed_uchar_missing_nodefault`, `class_missing_nodefault`), several (clang lists them in one diagnostic, GCC one per
  enumerator), a switch with no case at all (`unfixed_empty_switch`, `unfixed_no_case_stmt_nodefault`; GCC adds `-Wswitch-unreachable` for a statement before the first label), an enumerator whose value no case has (`dup_enum_A_only_nodefault`: `enum dup { A = 0, B = 0, C }` with only `case A:` leaves `C`).
- Both rules fire for every spelling of the controlling expression that has the enumeration's type: a parameter, a local, a `typedef` name (including a `typedef` of an unnamed enumeration), a `const`-qualified parameter, a
  call result, a parenthesized expression, an assignment, a cast `(enum color)i`, and the inner switch of nested switches.

**What every compiler and mode accepts:**

- A `default` label (any missing enumerator, any `case 4`: `unfixed_missing_blue_default`, `unfixed_only_default`, `unfixed_all_and_default`) silences rule 2; it does not silence rule 1 (`unfixed_case4_default` warns).
- All enumerators covered, without `default` (`unfixed_all_nodefault`, `neg_enum_all`, `fixed_uchar_all_nodefault`, `fixed_ulong_all_nodefault`, `class_all_nodefault`, `fallthrough_all_nodefault`).
- A case value that is an integer expression equal to an enumerator's value (`case RED + 1:`, `unfixed_case_red_plus_1`; `case 1u:`; `case 2:` where it is `BLUE`), and enumerators that share a value (`dup_enum_*`: covering one of two
  aliases covers both).
- **A controlling expression that is not of enumeration type:** `switch ((int)c)`, `switch (c + 0)`, `switch (+c)` (`int_cast_case4`, `int_cast_enumerators_missing`, `promoted_c_plus_0_case4`, `switch_unary_plus_case4`): no rule applies, so
  `case 4:` and missing enumerators are accepted.

**Where they disagree** (6 shapes; the `ok` cells are `-Wall -Wextra -Werror` clean; sources in `matrix/`):

| shape | clang C23 | clang C++20 | gcc C23 | g++ C++20 |
|---|---|---|---|---|
| `switch (RED) { case RED: ...; case GREEN: ... }` (an enumerator, `BLUE` missing; `switch_on_enumerator_directly_missing`) | ok | ok | ok | **warns** (`BLUE` not handled) |
| `switch (RED) { case 0: ...; case 4: ...; default: }` (`switch_on_enumerator_case4`) | ok | **warns** | **warns** | **warns** |
| `switch (i ? RED : GREEN) { case RED: ... }` (`switch_ternary_missing`) | ok | **warns** | **warns** | **warns** |
| `switch (i ? RED : GREEN) { case 0: ...; case 4: ...; default: }` (`switch_ternary_case4`) | ok | **warns** | **warns** | **warns** |
| `const enum color K = RED; switch (K) { case RED: ... }` (`switch_const_obj_missing`) | ok | ok | **warns** | **warns** |
| `switch (OA) { case OA: ... }` (the other enumeration's enumerator, `OB` missing; `switch_other_enumerator_missing`) | ok | ok | ok | **warns** |

Reading the table: in C an enumerator has type `int`, so clang C23 (and GCC C for a bare enumerator) treat the expression as an integer; clang folds a `const` object of enumeration type to its value; GCC keeps its enumeration
type. **g++ warns on all six; it is the strictest.** Smaller differences that do not change a verdict: the diagnostic wording (`enum color` against `color`, one message per enumerator against a list), `-Wenum-compare-switch` in clang
C++ for a `case` of another enumeration (`unfixed_case_other_enum`: every compiler also reports `duplicate case value`, an error), and clang C++20's "not a constant expression" for `case (enum color)7:`.

**The owner's rule (2026-10-22, "strictest, as g++").** The two rules apply **in both modes** to a switch whose controlling expression, before promotion, is an enumeration value, an enumerator, or a `?:` whose arms are enumerators of one
enumeration (g++'s reading, which includes a `const` object of enumeration type). They do not apply to a controlling expression that is an integer expression (a cast to `int`, `c + 0`, `+c`, an ordinary `int`).
Because an enumerator in C is an `int` constant (P25, P26), the encoder has to recognize an enumerator, or a `?:` of them, as an enumerator of that enumeration in C by the node the front end leaves (P25's `ENUM:` constants), not by
the type of the expression.

### P32: IL of an ordering comparison on enumerations: **PASS (one wording point for item 10, below)**

Evidence is under `build/m4.2c/probes/`: `p32.c` (functions `a1`-`a8` color, `b1`-`b3` neg, `c1`-`g2` fixed bases; the enumerations `color {RED, GREEN, BLUE}`, `neg {NA = -1, NB, NC}`, `other {OA, OB}`, `uc : unsigned char`, `si : int`,
`ui : unsigned int`, `sl : long`, `ul : unsigned long`), `p32b.c` (a negative-valued and an unsigned-compatible enumeration together, `m1`-`m4`), `p32_class.cpp` (`enum class`, `h1`-`h5`). IL: `p32.c.c23.il`, `p32.c.c++20.il`,
`p32b.c.*.il`, `p32_class.cpp.c++20.il`; condensed trees: `p32.c.c23.trees`, `p32.c.cxx.trees`, `p32_class.cpp.cxx.trees` (`il.py fn`; `[gen]` marks a compiler-generated node, `{expr=...}` a folded constant's `expr` chain, `ENUM:X [named_def]`
an enumerator). Both modes ran (`./fe.sh`): C23's only warning is #68 (a sign change) at `c < -1`, which verify mode suppresses.

**The type each comparison is computed in** (the type of the `eok_lt`/`eok_le` operands; both operands are `eok_cast [gen]` to it, a variable operand from its enumeration type):

| enumeration (values) | operand pair | C23 | C++20 |
|---|---|---|---|
| `color` 0..2, unfixed | value, value (`a < b`, `a2`) | **`unsigned int`** (the compatible type) | **`int`** |
| | value, enumerator (`c < BLUE`, `c <= GREEN`, `RED < c`) | `unsigned int` | `int` |
| `neg` -1..1, unfixed | `n < NC`, `a < b`, `n < 0` | `int` | `int` |
| `uc : unsigned char` | `a < b`, `a < U1` | `int` (the promoted base) | `int` |
| `si : int` | `a < b`, `a < I1` | `int` | `int` |
| `ui : unsigned int` | `a < b`, `a < V1` | `unsigned int` | `unsigned int` |
| `sl : long` | `a < b`, `a < L1` | `long` | `long` |
| `ul : unsigned long` | `a < b`, `a < W1` | `unsigned long` | `unsigned long` |
| `enum class S`, `T : unsigned char`, `N : int` | `a < b`, `a < S::SB` | n/a | **no conversion node**: `eok_lt[bool](var a:S, var b:S)`, a built-in comparison of the scoped enumeration itself (`p32_class.cpp.cxx.trees`) |

The result type differs only by the front end's own wrapping: C `eok_lt[int](...)`; C++ `eok_cast[int](eok_lt[bool](...)) [gen]`.

**The `expr` chain of an enumerator operand** (an enumerator is an `int` constant in C and a constant of the enumeration type in C++; `p32.c.c23.trees`, `p32.c.cxx.trees`):

| source | C23 | C++20 |
|---|---|---|
| `c < BLUE` (`a1`) | `const unsigned int 2U {expr=eok_cast[unsigned int](const int 2 ENUM:BLUE [named_def]) [gen]}` | `const int 2 {expr=eok_cast[int](const color 2 ENUM:BLUE [named_def]) [gen]}` |
| `RED < c` (`a7`) | `const unsigned int 0U {expr=eok_cast[unsigned int](const int 0 ENUM:RED [named_def]) [gen]}` | `const int 0 {expr=eok_cast[int](const color 0 ENUM:RED [named_def]) [gen]}` |
| `n < NC` (`b1`, neg) | `const int 1 {expr=eok_cast[int](const int 1 ENUM:NC [named_def]) [gen]}` | `const int 1 {expr=eok_cast[int](const neg 1 ENUM:NC [named_def]) [gen]}` |
| `a < U1` (`c2`, fixed `uc`) | `const int 1 {expr=eok_cast[int](const enum uc 1U ENUM:U1 [named_def]) [gen]}`: **in C a fixed-base enumerator has the enumeration's type** | `const int 1 {expr=eok_cast[int](const uc 1U ENUM:U1 [named_def]) [gen]}` |
| `a < V1` (`e2`, fixed `ui`) | `const unsigned int 7U {expr=eok_cast[unsigned int](const enum ui 7U ENUM:V1 ...) [gen]}` | `const unsigned int 7U {expr=eok_cast[unsigned int](const ui 7U ENUM:V1 ...) [gen]}` |
| `RED < 2` (`a3`) | folded: `const int 1 {expr=eok_lt[int](const int 0 {expr=eok_cast[int](const int 0 ENUM:RED ...) [gen]}, const int 2)}`, **compared as `int`**: both operands are constants, the enumerator is `int` | folded: `const int 1 {expr=eok_cast[int](const bool 1 {expr=eok_lt[bool](const int 0 {expr=eok_cast[int](const color 0 ENUM:RED ...) [gen]}, const int 2)}) [gen]}` |

So in C a bare enumerator is `int` and an unfixed-enumeration value is converted to its compatible type; `c < BLUE` has an `int` constant against a `color` value and is computed in `unsigned int` (the compatible type of `color`) because
the value's type is the higher-ranked one. A fixed-base enumerator keeps the enumeration's type in C.

**Operands the record rejects** (item 10), what each does:

| source | C23 | C++20 | same values? |
|---|---|---|---|
| `c < 2` (`a4`) | `lt(cast<unsigned>(c), const unsigned int 2U {expr=cast<unsigned>(const int 2)})` | `lt(cast<int>(c), const int 2)` | yes for every in-range `c` (0..2), but not for a negative literal |
| `c < -1` (`a5`) | `lt(cast<unsigned>(c), const unsigned int 4294967295U {expr=cast<unsigned>(const int -1 {expr=negate(1)})})`: **true for every `c`** | `lt(cast<int>(c), const int -1 {expr=negate(1)})`: **false for every `c`** | **no** (the reason `c < -1` stays rejected) |
| `c < o`, two unfixed enumerations (`a6`) | `lt(cast<unsigned>(c), cast<unsigned>(o))` | `lt(cast<int>(c), cast<int>(o))` | yes for non-negative enumerations (both ranges are in 0..) |
| `n < c`, `c < n` (`neg` with `color`, `p32b.c` `m1`, `m2`) | `lt(cast<unsigned>(n), cast<unsigned>(c))`: **`n = -1` becomes 4294967295** | `lt(cast<int>(n), cast<int>(c))` | **no** (`n = -1`: C false, C++ true) |
| `c < 0u`, `n < 0u` (`m3`, `m4`) | `unsigned int` | `unsigned int` | yes |

**Do both modes compare the same values for every in-range pair of one enumeration (or an enumeration value and one of its enumerators)? Yes**, for all seven enumerations probed: where the type differs (`color`: `unsigned int` in C, `int` in C++) every value of
the enumeration is in 0..2, which both types hold, so the conversion keeps each value and the order; every other row has the same type in both modes. The cases that differ are exactly the mixed ones: an enumeration against a negative literal
(`c < -1`) and a negative-valued enumeration against an unsigned-compatible one (`n < c`), both of which the record keeps rejected.

**Wording to correct in item 10.** The record says both operands are converted "in C++, `int`". The IL shows **in C++ the promoted underlying type**: `int` for an unfixed enumeration and for bases up to `int` (`uc`, `si`), but `unsigned int` for `: unsigned int`,
`long` for `: long` and `unsigned long` for `: unsigned long` (`e1`-`g2`), the same as C for those. The conclusion (one meaning in both modes for one enumeration) is unaffected, because a value of the enumeration lies in the value range, which both types
hold. Scoped enumerations are compared as themselves (no conversion node) and are outside item 10 as written (unscoped only).

**Wording to correct in item 11.** It says the rules apply to "a switch whose controlling expression has an enumeration type before promotion". In C an enumerator and a `?:` of enumerators have type `int` (P25), so that wording would not cover the
shapes on which the owner's rule ("strictest, as g++", P31) applies in both modes; the record should state the rule as the owner gave it: the controlling expression is an enumeration value, an enumerator, or a `?:` of enumerators of one enumeration (a `const`
object of enumeration type is an enumeration value, as for g++).

*2026-10-20: milestone numbers before the M4.1 section follow the renumbering of that date (plan.md): generic templates are M5.*
