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
- *M4.2c (2026-10-22, pin `f0ed5525af`):* the caveat below, a spurious #61 on a valid 128-bit multiply
  (`(__int128)9223372036854775807LL * 4`), is gone: upstream #38 folds `__int128` constants with 128-bit limits.
  `p9_wide_constants.cpp` is `frontend-ok` now (owner), and `p9_wide_constants_overflow.cpp` pins both sides: #61 for
  a product outside `__int128` (2^127, 2^128), none for one inside (2^65, 3 * 2^125) or for an unsigned wrap.
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

### P30: the front end's internal error #2656 on a suppressed note: **PASS** (cause found; fixed on fork branch `0022`, pinned at `f0ed5525af` the same day)

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
  `edg_bugs/upstreamed/0022-suppressed-note-internal-error.md`.
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


## M4.3a: side effects in expressions

Probes P33-P35 and spikes S17-S18 for decision 0020. Evidence is under `build/m4.3a/probes/` and `build/m4.3a/spikes/` (gitignored). The IL probes ran as
`scripts/cerastes-fe.sh --c23|--c++20 --display_error_number --il_display <probe>` (`build/m4.3a/fe.sh`: output `<probe>.<mode>.il` / `.err`); `build/m4.3a/tree3.py <il> fn [NAME...]` condenses
a function body to a tree (`lv` = `is_lvalue`, `nu` = `result_is_not_used`, `par` = `is_parenthesized`, `gen` = `compiler_generated`, `olt` = `orig_lvalue_type`; `POS=1` adds the position and range);
the trees are `probes/p33*.trees`. `build/m4.3a/warn.py` ran each probe through the plain front end and `build/cerastes verify` (verify mode's flags), `build/m4.3a/matrix.py` ran the native compilers
(`matrix.json`, `matrix.txt`; the tables below come from `build/m4.3a/tables.py`). Silicon was run as `scripts/run_tests.py` runs `.vpr` fixtures (`spikes/gen17.py`, `spikes/gen18.py` write the `.vpr` and
`.out` files and run `java -Xss128m -jar silicon.jar --z3Exe ... --numberOfErrorsToReport 0`). `il_def.h` lines are in `third_party/edg/src/il_def.h` at the pin `f0ed5525af`. Probe files, one function per
line: `p33.c|cpp` (`a1`-`a25`), `p33b.c|cpp` (`c1`-`c34`), `p33c.cpp` (`d1`-`d12`, C++ lvalue uses), `p33d.c` (`u1`-`u7`, unevaluated C), `p33e.cpp` (`e1`-`e6`, unevaluated C++), `p34.c|cpp`
(`u*` unsequenced, `k*` commas, `h*` conditions, `d*` discarded values, `s*` unevaluated), `p34b.c|cpp` (`j*`, assignments of a constant as a condition), `p34c.c|cpp` (`w*`, writes of the target inside the
right-hand side), `p34d.c|cpp` (`m*`, comma and discarded-value extras). The `.cpp` files are copies of the `.c` files (`typeof` becomes `decltype` in `p34.cpp`); every probe ran in both modes.

### P33: IL of side effects used as values: **PASS (no stop rule triggered)**

The stop rules (C++ wrapper node the encoder cannot map; value not the converted value; node type not the left operand's; `parenthesized` unreliable) were each checked and none fires. The field is
`is_parenthesized` (`an_expr_node` :15075, "TRUE if the expression was parenthesized in the source code"); there is no `eok_parens` node (`PARENS_IN_IL` is 0).

**1. A used `=`, `op=`, `++`, `--` is its own node and sits directly as the operand; there is no wrapper in either mode.** Its `type` is the left operand's type; a conversion to the context's type is a separate
`compiler_generated` `eok_cast` (or `eok_bool_cast`) around it, never between the lvalue and the node. `probes/p33.c.c23.trees`, `p33.cpp.cxx.trees`, `p33b.*.trees`:

```text
a1  stmk_init [dik_expression] var=y eok_add[int](eok_assign[int] {par}(var x:int {lv}, var a:int {olt=int}), const int 1[int])                       (C23)
a1  ... eok_add[int](eok_assign[int] {par,olt=int}(var x:int {lv}, var a:int {olt=int}), const int 1[int])                                           (C++20)
a3  stmk_expr eok_assign[int] {nu}(var i:int {lv}, eok_cast[int] {gen}(eok_assign[short](var s:short {lv}, var v:short {olt=short})))                 (C23; C++20: the same, {lv,nu} / {olt=short})
c16 ... eok_add[int](eok_cast[int] {gen}(eok_add_assign[short] {par}(var s:short {lv}, const int 1000[int])), var a:int {olt=int})                     (compound: short type, rhs int)
c18 ... eok_add[int](eok_cast[int] {gen}(eok_post_incr[short](var s:short {lv})), var a:int {olt=int})
c20 stmk_return eok_cast[short] {gen}(eok_post_incr[int](var x:int {lv}))                          (return x++ in a short function)
a20 stmk_expr eok_assign[bool] {nu}(var b:bool {lv}, eok_bool_cast[bool] {gen}(eok_assign[int] {par}(var x:int {lv}, const int 5[int])))              (b = (x = 5))
a21 stmk_init [dik_expression] var=y eok_cast[int] {gen}(eok_assign[bool] {par}(var b:bool {lv}, const bool 1U[bool]))                                (int y = (b = true))
a22 ... eok_eq[int](eok_cast[unsigned int] {gen}(eok_assign[enum color] {par}(var c:enum color {lv}, const enum color 1U {expr=...}[enum color])), ...)  (C23; C++20: cast to int, color)
c21 stmk_expr eok_assign[long] {nu}(var l:long {lv}, eok_cast[long] {gen}(eok_assign[int](var i:int {lv}, const int 5[int])))                          (l = i = 5)
```

The value of the node is the new value of the left operand (the converted right-hand side); `x++`/`x--` (`eok_post_incr`/`eok_post_decr`, `il_def.h:14051`) are prvalues of the left operand's type that yield the old value.
`a2` (`y = x = a`) is `eok_assign[int](y, eok_assign[int](x, a))` in both modes with no node between; a chain across types (`a3`, `c21`) has exactly one `eok_cast {gen}` where the types differ.
Every right-hand-side variable read is `olt=<type>` (glvalue-to-prvalue by clearing `is_lvalue`, no node: `il_def.h:14907`); no read of a written variable is wrapped.

**2. C23 against C++20 (`is_lvalue` :14942, `orig_lvalue_type` :14907).**

| node | C23 | C++20 |
|---|---|---|
| used `x = e`, `x op= e`, `++x`, `--x` | `type` = left's type, no `lv`, no `olt` | `type` = left's type, **no `lv`, `olt` = that type** (the lvalue cleared) |
| used `x++`, `x--` | prvalue, no flags | prvalue, no flags |
| discarded `x = e`, `++x`, `op=` (statement, comma's left operand, `for` init) | `{nu}` | `{lv,nu}` |
| discarded `x++`, `x--` | `{nu}` | `{nu}` |
| comparison, `!`, `&&` | type `int` | type `bool` |
| `if`/`while`/`for`/`do` condition that is an assignment or an int | the assignment itself | `eok_bool_cast[bool] {gen}` around it |
| `?:` condition | the operand itself | `eok_bool_cast[bool] {gen}` around it |

An assignment whose result is used **as an lvalue** (C++ only) keeps `is_lvalue`: `(x = 1) = 2` (`d1`: `eok_assign {lv,nu}(eok_assign[int] {lv,par}(x, 1), 2)`), `++x = 3` (`d2`), `++++x` (`d3`), `(a, x) = 5` (`d4`: the comma
is `{lv,par}`), `(c ? x : y) = 1` (`d5`), `(x += 1) += 2` (`d7`), `int& r = (x = 1)` (`d6`, `eok_reference_to {gen}`). `lvalue_var` (`encode.cpp:1670`) rejects any left operand that is not an `enk_variable`, which covers all of them
(`probes/p33c.cpp.cxx.trees`). `(x) = 3` and `(x)++` are `enk_variable {lv,par}` and pass `lvalue_var`.

**3. `result_is_not_used`** (:14963) is on the top node of an expression statement, on a comma and its **left** operand, on a `(void)` cast and its operand (`c13`: `eok_cast[void] {nu}(eok_assign {nu,par}...)`), and on a `for`
step expression. It is **not** on the right operand of a discarded comma (`a15`, `for (i = 0, j = n; ...; i++, j--)`: init `eok_comma[int] {nu}(eok_assign {nu}(i, 0), eok_assign(j, n))`, step
`eok_comma[int] {nu}(eok_post_incr {nu}(i), eok_post_decr(j))`; the right operand carries no `nu` in either mode). The encoder must take "discarded" from the context, not from `nu` on the right operand.

**4. The comma** is `eok_comma` (:14174), `type` = the right operand's type (`v(), x++` is `eok_comma[int]`; `v(), v()` is `eok_comma[void]`), with a `(void)` call as left operand as `eok_cast[void] {nu}(eok_call {nu}(...))`
(`a17`). A comma whose left operand has no effect has the bare operand with `nu` (`c24` C23: `eok_comma[int] {par}(var a:int {nu,olt=int}, const int 3)`; C++20 `var a {lv,nu}`). A used comma with parentheses
has `par` (`a16`: `eok_comma[int] {par}(eok_assign {nu}(x, a), eok_add(x, 1))`); in C++20 a used comma of an int value has `olt=int` (`c26`). `(x = 1, x = 2)`: left `{nu}`/`{lv,nu}`, right the plain assignment.

**5. Conditions and `is_parenthesized`.** `par` sits on the assignment, never on the `eok_bool_cast` or on the comparison around it. It is reliable (`probes/p33b.*.trees`, `c1`-`c12`):

| source | C23 condition | C++20 condition |
|---|---|---|
| `if (x = a)` | `eok_assign[int](...)`, no `par` | `eok_bool_cast {gen}(eok_assign {olt=int}(...))`, no `par` |
| `if ((x = a))` | `eok_assign {par}` | `eok_bool_cast {gen}(eok_assign {par,olt=int})` |
| `if (((x = a)))` | `eok_assign {par}` | same |
| `if ((x = a) != 0)` | `eok_ne[int](eok_assign {par}, 0)` | `eok_ne[bool](eok_assign {par,olt=int}, 0)` |
| `while (x = a)` / `while ((x = a))` (and `for`, `do`) | no `par` / `par` | `eok_bool_cast {gen}(...)` with the same `par` |
| `!(x = a)`, `(x = a) && b`, `b && (x = a)`, `c ? (x = a) : 0` | `par` on the assignment | the same, under the casts |
| `if (x = a, x > 0)` | `eok_comma[int](eok_assign {nu}, eok_gt)`, no `par` | `eok_comma[bool](eok_assign {lv,nu}, eok_gt[bool])` |

So `assignment_as_condition` is: the whole condition of an `if`/`while`/`for`/`do`, after peeling one `eok_bool_cast {gen}` (C++), is an assignment or compound assignment node without `is_parenthesized`.
The statement's own `(`..`)` is not recorded as parentheses (`c1`, `c5`: no `par`), so no false negative; a double parenthesis is present (`c2`, `c3`, `c6`, `c7`).

**6. Statements.** `stmk_while`/`stmk_for`/`stmk_end_test_while` keep the condition in `expr` (`stmk_for`: `initialization` a statement, `increment` an expression, body in `statement`; loops: `loop_statement`);
C23 wraps each loop in a `compiler_generated` `stmk_block`, C++20 does not (`a13`, `a14`, `a15`). `stmk_switch` `expr` is the `eok_post_incr` itself (`a9`, no cast, both modes). `eok_question` has the arms as the
effect nodes directly (`a19`: `eok_question[int](var c, eok_post_incr, eok_post_decr)`; C++20 wraps the condition in `eok_bool_cast {gen}` and puts `olt=int` on the `?:` and on its used arms, `c32`, `c33`).
`return (x = a, x + 1)` (`a16`) and `return x++` (`a8`) have the effect node directly under `stmk_return`; an `int y = (x = a) + 1;` initializer is a `stmk_init` whose `dynamic-init` has `kind dik_expression` and
`expression` the tree (the `dik_constant` form is only for constant initializers).

**7. Positions.** Every operation node's `position` is the operator token and `expr_range` spans its operands without enclosing parentheses (`POS=1 tree3.py`: `a1` the assign `@5:35[33-37]` for `x = a` at columns 33-37;
`a8` `x++` `@12:36[35-37]`; comma `@19:64[61-68]`). A diagnostic at an `AssignE`'s IL node therefore points at the `=` / `++` / `,`.

**8. Unevaluated operands.** `sizeof(x++)` is folded to a constant (`const int 4 {expr=eok_cast {gen}(const unsigned long 4UL {expr=enk_sizeof}...)}`, `a23`, `u1`, `u2`, `s2`); the operand node survives **only**
in the constant's `expr` chain: the constant's `expr` -> `enk_sizeof` node (:13611, `is_type FALSE`) -> `expr` = `eok_post_incr {par}` (`p33.c.c23.il`, function `a23`). `sizeof(x = 1)`, `sizeof x++` the same. `typeof(x++)` (C) and
`decltype(x++)` (C++) leave **no expression in any statement or expression tree**: the operand is a `local-expr-node-ref` entity with `decltype referrer: <type>` (`p33d.c.c23.il` function `u3`, `p33e.cpp.c++20.il`
function `e2`), reachable only from the type. `noexcept(x++)` is a folded constant whose `expr` is `eok_noexcept(eok_post_incr)` (`e3`). A walk of the statement trees for rule (c) sees `sizeof` operands through the
constant chain only; it cannot see `typeof`/`decltype` operands at all (they are not evaluated, so this is sound; it only matters if the owner wants `typeof(x++)` rejected, P35).

**Stop rule for the interface (`build/m4.3a/vir-api.md`):** nothing in P33 contradicts it. `AssignE.type` = the left operand's type holds for every node above; `yields_old` for the post forms; the converted value of a
narrowing chain is the inner node's `type` followed by the encoder's own conversion (the gen cast), exactly as for `assign` today.

### P34: EDG's own warnings on these shapes: **UNCLEAR for the owner's suppression rule (no EDG number qualifies for suppression); no EDG sequencing warning exists**

`build/m4.3a/warn.py` runs each probe through plain `scripts/cerastes-fe.sh --display_error_number` and through `build/cerastes verify` (at the baseline binary, before the M4.3a encoder; verify mode prints EDG's warnings, then
cerastes's own errors), in both modes. The two agree on every warning number: verify mode adds only the `cerastes-*` errors (`unsupported_expression`, `call_without_contract`, `missing_decreases`, which are
today's rejections of these shapes). Verify mode's suppression list is `src/driver/verify_main.cpp:313`: `--diag_suppress=61,62,63,39,179,68,69,188,546,940,1305,2803`, plus `--no_use_before_set_warnings` (#549), C only
`--diag_suppress=257` and `--diag_error=1901`, and `--diag_error=161,1675`. So **#174, #187, #550, #177, #111 are not suppressed today** and print, with exit status decided by verification (`probes/t2.c`, `x + 1;`: the #174
warning, then `error: overflow`, exit 1). The "EDG (C / C++)" column of the tables below lists the numbers other than #550/#177/#68 (variables set but not used, declared but not referenced, integer sign change: probe artefacts).

- **No sequencing warning.** Not one of `u1`-`u19`, `w1`-`w24` (all the unsequenced shapes) gets an EDG diagnostic of any number. cerastes's `unsequenced_modification` is the only report.
- **#174 "expression has no effect"** (verify mode: not suppressed). Fires on: a comma's left operand that has no effect (`(a, b);` `k1`, `y, x++;` `k3`, `(a, 3)` `k4`/`c24`, `(a, x++)` `k9`, `for (...; a, x++)` `k11`, `(x, 1);` `k15`,
  `(1, x++)` `w8`, `(void)(a, b)` `m2`), **and on shapes the plan accepts or treats separately**: the pure discarded statements `x + 1;` (`d4`) and `+x;` (`d14`, 0012 item 3 keeps them accepted), and `sizeof(x++);` as a statement (`d6`).
  It does **not** fire on the (a) shapes (`x++, y;` `k2`, `x++, 1;` `k8`, `x = 1, y;` `m10`, `(void)g(a), 1;` `m12`, a `for` init `x = 0, a;` `k10`), on (b) (`x++ + 1;` `d1`, `x == (y = 1);` `d2`, `(x = 1) + 0;` `d3`, `x++ < 3;`,
  `-x++;`, `!x++;`), or on (c) in an expression (`sizeof(x++)` in `s1`/`s2`; `typeof`/`decltype`). So #174 covers `x + 1;`, which cerastes accepts: **it cannot be suppressed under the owner's rule**, and cerastes's
  `comma_without_effect` is needed for (a) regardless.
- **#187 "use of = where == may have been intended"** (not suppressed). Fires **only when the right-hand side is a constant** (`if (x = 3)`, `if (x = 'c')`, `if (x = 1 + 2)`, `while`, `do`, `for`: `j1`, `j2`, `j10`, `j11`, `j13`, `j14`, `j17`, `j20`);
  never for `if (x = a)`, `if (x = a + 1)`, `if (x = g(a))`, `if (x += 1)`, `if (x = y = 3)` (`h1`..`h5`, `j3`, `j4`, `j6`, `j16`). It **also fires on shapes the compilers accept silently and the plan accepts**: `if ((x = 3))` (double parentheses,
  `j5`), `(x = 3) && a` (`j7`), `a && (x = 3)` (`j8`), `(x = 3) ? a : x` (`j9`), `(x = 1) ? x : 0` (`w16`), `!(x = 3)` (`j12`), `a == 1 || (x = 3)` (`j18`), and the statements `a && (x = 1);` / `a || (x = 1);` (`m17`, `m18`). EDG's rule ignores
  the extra parentheses. **#187 cannot be suppressed** (it covers accepted shapes), and it does not cover the non-constant `if (x = a)`: cerastes's `assignment_as_condition` is needed anyway. A test whose accepted shape has a
  constant right-hand side (`a && (x = 1);`) prints #187 as a warning and still verifies; tests that use a non-constant right-hand side print nothing.
- **#550** ("set but never used"): fires on a local or parameter that is only written (`u6`, `u12`, `h7`-`h9`, `h16`, `a10`, `a18`); the plan's positives read every written variable. **#177** (declared but never referenced, `c25`),
  **#111** (unreachable statement, `j15`: a consequence of `if (x = a, 1) return x; return a;`) and **#68** (sign change in `u += -1`, `c17`, already suppressed) are artefacts. **#549** is off.

**Verdict.** No EDG warning number can be added to the suppression list under the rule "suppress only when cerastes rejects every shape the number covers": #174 covers `x + 1;`, #187 covers `if ((x = 3))` and the other
accepted shapes, #550 and #177 stay by decision (handoff §11 question 20, as recommended). The plan's stop rule 2 ("an EDG number covers a shape cerastes accepts") applies to #174 and #187 only as a reason **not to suppress**;
both already print today for accepted code (`x + 1;`). For the owner: with no suppression, a rejected `if (x = 3)` prints #187 and then `assignment_as_condition` (two messages for one problem); every other rejected shape
prints only the cerastes message.

### P35: native compilers on the same shapes: **PASS (matrix below; the compilers disagree on several shapes, the union is stated)**

`build/m4.3a/matrix.py` compiles each probe with Apple clang (Command Line Tools, `$CERASTES_CLANG_CC`/`_CXX`) and GCC 16 (Homebrew `gcc-16`, `g++-16`), `-fsyntax-only -Wall -Wextra` (no `-Werror`), `-x c -std=c23` and `-x c++ -std=c++20`;
`matrix.txt` is the raw table, `matrix.json` and `matrix_detail.json` (messages) the data. 211 shapes. A cell is the warning flag, `-` for none; the flags `-Wunused-parameter`, `-Wunused-variable`,
`-Wunused-but-set-variable|parameter` (artefacts of a probe that writes without reading) are left out of the cells, except that a positive test must read every variable it writes (`-Wunused-but-set-variable` is in `-Wall`).
The last column is EDG (P34). Shapes that are silent everywhere (115) follow the table.

**Every P33 shape is silent in all four configurations**, except the two that are the (b) and (c) shapes by design: `a23` `sizeof(x++)` (clang only) and `a24` `x++ + 1;` (all). `a1`-`a22`, `a25`, `c2`-`c4`, `c6`-`c23`, `c26`-`c34` and
`d1`-`d12` (C++ lvalue uses) produce no warning of any kind (`c25`, `c1`, `c5`, `c24` are the expected ones: unused variable, `if (x = a)`, `while (x = a)`, `x = (a, 3)`; `p33c:d4`, `(a, x) = 5`, warns `-Wunused-value` in both C++ compilers: the comma's left operand `a` has no effect).

**What the compilers agree on, and the union (the owner's rule: strictest, in both modes).**

- **`-Wparentheses`** warns, in all four, on an assignment that is the whole condition of `if`/`while`/`do`/`for` without extra parentheses: `if (x = a)`, `while (x = a)`, `do {} while (x = a)`, `for (; x = a;)`, with any right-hand side
  (`h1`, `h2`, `h4`, `h5`, `j1`-`j4`, `j10`, `j11`, `j13`, `j14`, `j16`, `j17`, `j20`). It is **silent in all four** on: `if ((x = a))` and `(((x = a)))` (`c2`, `c3`, `j5`), `if ((x = a) != 0)`, `!(x = a)` (`h6`, `j12`), `(x = a) && b`,
  `b && (x = a)`, `a || (x = a)`, `(x = a) ? 1 : 2`, `c ? (x = a) : 0` as a condition, `a && (x = a)`, `x = (x = a) ? 1 : 2`, `if (x = a, x)` (a comma), `if (x++)`, `if (x += a)` and `if (x -= a)` (compound assignment other than bitwise:
  `h3`, `h15`, `j6`). **Only clang** warns on `if (x |= a)` (`h13`; bitwise assignment is rejected anyway until M4.4). The set to reject, with the union, is the whole condition being `=` (and the bitwise compounds) with no `par`; compound `+=` and `-=` are not warned by either compiler.
- **`-Wunused-value`** (both, with clang's `-Wunused-comparison` for a discarded comparison, which GCC reports as `-Wunused-value`): a comma whose left operand has no effect (`k1`, `k3`, `k4`, `k9`, `k11`, `k15`, `m2`: all four); **a discarded comma whose right operand
  has no effect** (`x++, y;` `k2`, `x++, 1;` `k8`, `x = 1, y;` `m10`, `(void)g(a), 1;` `m12`, the `for` init `x = 0, a;` `k10`: all four, shape (a)); a discarded operator with an effect inside (shape (b)): `x++ + 1;` `d1`/`a24`, `x * 2` `m23`,
  `(x += 1) + 1;` `m24`, `-x++;` `d7`, `!x++;` `m15`, `x == (y = 1);` `d2`, `x++ < 3;` `d9`, `x++ == 1;` `m14`, `(x = 1) == 1;` `d8`, `(x = 1) == y;` `m22`: all four; **gcc C is silent on** `(x = 1) + 0;` (`d3`, `m21`), **both GCCs are silent on** `(int)x++;` (`d10`),
  **g++ is silent on** `(x = 1, x + 1);` and `(x++, x + 1);` (`d17`, `d18`). The pure `x + 1;` (`d4`), `+x;` (`d14`) and `g(x) + 1;` (`d13`) warn in all four (0012 item 3 accepts `x + 1;`; the plan keeps it accepted). **Silent in all four**, so accepted: `(void)(x++ + 1);` (`d5`, `m13`),
  `(void)x++;`, `x++, (void)y;` (`m1`), `(void)(x++, y);` (`m3`), `(void)y, x++;` (`k7`), `v(), v();`, `g(a), x++;`, `(void)0, x` (`k12`-`k14`), `x++, x++;` (`m8`), `(x = 1, y = 2);` (`m9`), `y = (x++, y)` (`m6`), `a ? x++ : 0;`, `a ? x++ : x--;`
  (`d11`, `m20`), `a ? (x = 1) : (x = 2);` (`m19`), `a || (x = 1);`, `a && (x = 1);` (`m17`, `m18`), `(x = 1);`, `(x++);` (`m28`, `m29`), `((void)0, x++);` (`m30`). **`a && x++;` warns only in gcc C** (`d12`: "value computed is not used"): by the union
  rule it is rejected in both modes, while `a && (x = 1);` is accepted (an asymmetry of GCC's, recorded for the owner).
  `x = x;` (`d16`) warns `-Wself-assign` in clang only; the plan does not list it.
- **`-Wunsequenced` (clang) / `-Wsequence-point` (GCC)**, the shapes where the compilers differ are the C++17 ones. **All four** warn on: `i++ + i`, `x + x++`, `x++ + x++`, `x++ * ++x`, `f(x++, x)`, `(x = 1) + x`, `(x = 1) + (x = 2)`, `(x = b) * (x = b)`,
  `(c ? x++ : 0) + x`, `int y = x++ + x`, `g(x++) + x`, `(x++ < 3) + x`, `x = x + (x = 1)`, `y = x++ + (x = 1)`, `g(x = 1) + g(x = 2)` (`u1`, `u4`-`u7`, `u9`-`u13`, `u16`, `u17`, `w11`, `w12`, `w15`). **Clang C, gcc C and g++ but not clang C++** (C++17 sequences the
  right operand of `=`/`op=` before the assignment): `x = x++` (`u2`), `x += x++` (`u3`: gcc C and clang C only), `x = (x = 1)`, `x = ++x`, `x = (x += 1)`, `x = (c ? x++ : 0)`, `x = x++ + 1`, `x = c && x++` (`w1`, `w2`, `w3`, `w5`, `w6`, `w9`), `x += (x = 1)` (`w7`: clang C and gcc C).
  **Shapes with a comma:** `(x++, 0) + x` (`u8`) and `(x++, (void)0, 3) + x` (`m4`) are warned by both clang modes and g++ but not by gcc C; `(x++ , 0) + (x++ , 0)` (`w21`) by both clang modes only. **No compiler warns**, although every one is
  well-defined: `x = x + 1`, `(x = 1, x) + 1`, `x++ && x`, `x++, x` (`u14`, `u15`, `u18`, `u19`), `x = (x++, 2)` (`w4`), `x = g(x++)` (`w14`), `y = (x = 2, x++)` (`w10`), `y = x = x + 1` (`w13`), `(x = 1) ? x : 0` (`w16`), `x++ ? x : 0` (`w18`), `int y = x++, z = x` (`w19`),
  `(x = 1) + (a = 2)` and `(x = 3) + a` (different variables: `w20`, `w24`), `(c ? x++ : x--) + 1`, `c ? x++ : (x = 2)` (`w22`, `w23`; one arm only is evaluated). The union covers all eight shapes of the plan's `unsequenced_modification` list. The plan's rule ("a write of the target inside the right-hand side of
  an assignment is a conflict") is stricter than every compiler on `x = (x++, 2)` and `x = g(x++)` (and on `x = (x++, 2)` the C standard agrees that it is defined): a reject with no compiler behind it; the owner may want to know.
- **Unevaluated operands** (`-Wunevaluated-expression`, clang only; GCC has no such warning): `sizeof(x++)`, `sizeof(x = 1)`, `sizeof x++` in C and C++ (`a23`, `u1`, `u2`, `s1`, `s2`, `d6`); **clang C++ only**: `decltype(x++)`, `noexcept(x++)`, `sizeof(decltype(x++))`, `sizeof(x++) + sizeof(x--)` (`e1`-`e4`, `e6`, `s3`, `s4`).
  `typeof(x++)` in C23 is silent in clang and gcc (`u3`, `u4`: no warning), `sizeof(typeof(x++))` too; `alignof(int)` is silent. The union applied in both modes would reject `typeof(x++)` in C although neither C compiler warns (clang warns the C++ spelling `decltype(x++)`): the encoder could not see
  the operand anyway (P33 item 8).

**Applying the union in both modes** gives these rejections: assignment (or `|=`-style compound) as a whole condition without parentheses; a comma whose left operand has no effect; a discarded comma whose right operand has no effect (the left operand may be a `(void)` call or any operand with an effect);
a discarded result of a non-assignment, non-call, non-`(void)`, non-`?:`/`&&`/`||`-at-top operator that has an effect inside (`x++ + 1;`, `x++ == 1;`, `(x = 1) + 0;`, `(int)x++;`, `(x = 1, x + 1);`) and, by gcc C alone, `a && x++;`; a side effect in the operand of `sizeof`
(and, if the owner extends the rule to the C++ spellings, `typeof`, `decltype`, `noexcept`); and every unsequenced-modification shape in the table, including the C++17-sequenced ones. Where the compilers warn on `a && x++;` but not on `a && (x = 1);` the rule has to pick one: the union rejects both only if the owner treats an
assignment under `&&` like an increment (GCC itself does not).

| shape | clang C | clang C++ | gcc C | gcc C++ | EDG (C / C++) |
|---|---|---|---|---|---|
| `p33:a23` `return sizeof(x++);` | unevaluated-expression | unevaluated-expression | - | - | - / - |
| `p33:a24` `x++ + 1; return x;` | unused-value | unused-value | unused-value | unused-value | - / - |
| `p33b:c1` `if (x = a) return x; return 0;` | parentheses | parentheses | parentheses | parentheses | - / - |
| `p33b:c5` `while (x = a) { a--; } return x;` | parentheses | parentheses | parentheses | parentheses | - / - |
| `p33b:c24` `x = (a, 3); return x;` | unused-value | unused-value | unused-value | unused-value | 174 / 174 |
| `p34:u1` `return i++ + i;` | unsequenced | unsequenced | sequence-point | sequence-point | - / - |
| `p34:u2` `x = x++; return x;` | unsequenced | - | sequence-point | sequence-point | - / - |
| `p34:u3` `x += x++; return x;` | unsequenced | - | sequence-point | - | - / - |
| `p34:u4` `return f(x++, x);` | unsequenced | unsequenced | sequence-point | sequence-point | - / - |
| `p34:u5` `return (x = 1) + x;` | unsequenced | unsequenced | sequence-point | sequence-point | - / - |
| `p34:u6` `return (x = 1) + (x = 2);` | unsequenced | unsequenced | sequence-point | sequence-point | - / - |
| `p34:u7` `return (c ? x++ : 0) + x;` | unsequenced | unsequenced | sequence-point | sequence-point | - / - |
| `p34:u8` `return (x++, 0) + x;` | unsequenced | unsequenced | - | sequence-point | - / - |
| `p34:u9` `return x + x++;` | unsequenced | unsequenced | sequence-point | sequence-point | - / - |
| `p34:u10` `return x++ + x++;` | unsequenced | unsequenced | sequence-point | sequence-point | - / - |
| `p34:u11` `return x++ * ++x;` | unsequenced | unsequenced | sequence-point | sequence-point | - / - |
| `p34:u12` `return (x = b) * (x = b);` | unsequenced | unsequenced | sequence-point | sequence-point | - / - |
| `p34:u13` `int y = x++ + x; return y;` | unsequenced | unsequenced | sequence-point | sequence-point | - / - |
| `p34:u16` `int y = g(x++) + x; return y;` | unsequenced | unsequenced | sequence-point | sequence-point | - / - |
| `p34:u17` `int y = (x++ < 3) + x; return y;` | unsequenced | unsequenced | sequence-point | sequence-point | - / - |
| `p34:k1` `(a, b); return b;` | unused-value | unused-value | unused-value | unused-value | 174 / 174 |
| `p34:k2` `x++, y; return x + y;` | unused-value | unused-value | unused-value | unused-value | - / - |
| `p34:k3` `y, x++; return x + y;` | unused-value | unused-value | unused-value | unused-value | 174 / 174 |
| `p34:k4` `return y;` | unused-value | unused-value | unused-value | unused-value | 174 / 174 |
| `p34:k8` `x++, 1; return x;` | unused-value | unused-value | unused-value | unused-value | - / - |
| `p34:k9` `return (a, x++);` | unused-value | unused-value | unused-value | unused-value | 174 / 174 |
| `p34:k10` `for (x = 0, a; x < 3; x++) { } return x;` | unused-value | unused-value | unused-value | unused-value | - / - |
| `p34:k11` `for (x = 0; x < 3; a, x++) { } return x;` | unused-value | unused-value | unused-value | unused-value | 174 / 174 |
| `p34:k15` `(x, 1); return x;` | unused-value | unused-value | unused-value | unused-value | 174 / 174 |
| `p34:h1` `if (x = a) return x; return 0;` | parentheses | parentheses | parentheses | parentheses | - / - |
| `p34:h2` `while (x = a) { a--; } return x;` | parentheses | parentheses | parentheses | parentheses | - / - |
| `p34:h4` `do { a--; } while (x = a); return x;` | parentheses | parentheses | parentheses | parentheses | - / - |
| `p34:h5` `for (; x = a; ) { a--; } return x;` | parentheses | parentheses | parentheses | parentheses | - / - |
| `p34:h13` `if (x \|= a) return x; return 0;` | parentheses | parentheses | - | - | - / - |
| `p34:d1` `x++ + 1; return x;` | unused-value | unused-value | unused-value | unused-value | - / - |
| `p34:d2` `x == (y = 1); return x + y;` | unused-comparison | unused-comparison | unused-value | unused-value | - / - |
| `p34:d3` `(x = 1) + 0; return x;` | unused-value | unused-value | - | unused-value | - / - |
| `p34:d4` `x + 1; return x;` | unused-value | unused-value | unused-value | unused-value | 174 / 174 |
| `p34:d6` `sizeof(x++); return x;` | unevaluated-expression,unused-value | unevaluated-expression,unused-value | unused-value | unused-value | 174 / 174 |
| `p34:d7` `-x++; return x;` | unused-value | unused-value | unused-value | unused-value | - / - |
| `p34:d8` `(x = 1) == 1; return x;` | unused-comparison | unused-comparison | unused-value | unused-value | - / - |
| `p34:d9` `x++ < 3; return x;` | unused-comparison | unused-comparison | unused-value | unused-value | - / - |
| `p34:d10` `(int)x++; return x;` | unused-value | unused-value | - | - | - / - |
| `p34:d13` `g(x) + 1; return x;` | unused-value | unused-value | unused-value | unused-value | - / - |
| `p34:d14` `+x; return x;` | unused-value | unused-value | unused-value | unused-value | 174 / 174 |
| `p34:d16` `x = x; return x;` | self-assign | self-assign | - | - | - / - |
| `p34:d17` `(x = 1, x + 1); return x;` | unused-value | unused-value | unused-value | - | - / - |
| `p34:d18` `(x++, x + 1); return x;` | unused-value | unused-value | unused-value | - | - / - |
| `p34:s1` `return sizeof(x = 1) + x;` | unevaluated-expression | unevaluated-expression | - | - | - / - |
| `p34:s2` `return sizeof x++ + x;` | unevaluated-expression | unevaluated-expression | - | - | - / - |
| `p34:d12` `a && x++; return x;` | - | - | unused-value | - | - / - |
| `p34:s3` `typeof(x++) z = 0; return z + x;` | - | unevaluated-expression | - | - | - / - |
| `p34:s4` `return sizeof(typeof(x++)) + x;` | - | unevaluated-expression | - | - | - / - |
| `p34b:j1` `if (x = 3) return x; return a;` | parentheses | parentheses | parentheses | parentheses | 187 / 187 |
| `p34b:j2` `while (x = 1) { a--; if (a < 0) break; } return x;` | parentheses | parentheses | parentheses | parentheses | 187 / 187 |
| `p34b:j3` `if (x = a + 1) return x; return 0;` | parentheses | parentheses | parentheses | parentheses | - / - |
| `p34b:j4` `if (x = g(a)) return x; return 0;` | parentheses | parentheses | parentheses | parentheses | - / - |
| `p34b:j10` `do { a--; } while (x = 0); return x + a;` | parentheses | parentheses | parentheses | parentheses | 187 / 187 |
| `p34b:j11` `for (; x = 2; ) { a--; if (a < 0) break; } return x;` | parentheses | parentheses | parentheses | parentheses | 187 / 187 |
| `p34b:j13` `if (x = 0) return x; return a;` | parentheses | parentheses | parentheses | parentheses | 187 / 187 |
| `p34b:j14` `if (x = -1) return x; return a;` | parentheses | parentheses | parentheses | parentheses | 187 / 187 |
| `p34b:j16` `int y = 0; if (x = y = 3) return x; return a;` | parentheses | parentheses | parentheses | parentheses | - / - |
| `p34b:j17` `if (x = 1 + 2) return x; return a;` | parentheses | parentheses | parentheses | parentheses | 187 / 187 |
| `p34b:j20` `if (x = 'c') return x; return a;` | parentheses | parentheses | parentheses | parentheses | 187 / 187 |
| `p34b:j5` `if ((x = 3)) return x; return a;` | - | - | - | - | 187 / 187 |
| `p34b:j7` `return (x = 3) && a;` | - | - | - | - | 187 / 187 |
| `p34b:j8` `return a && (x = 3);` | - | - | - | - | 187 / 187 |
| `p34b:j9` `return (x = 3) ? a : x;` | - | - | - | - | 187 / 187 |
| `p34b:j12` `if (!(x = 3)) return x; return a;` | - | - | - | - | 187 / 187 |
| `p34b:j15` `if (x = a, 1) return x; return a;` | - | - | - | - | 111 / 111 |
| `p34b:j18` `if (a == 1 \|\| (x = 3)) return x; return a;` | - | - | - | - | 187 / 187 |
| `p34c:w1` `x = (x = 1); return x;` | unsequenced | - | sequence-point | sequence-point | - / - |
| `p34c:w2` `x = ++x; return x;` | unsequenced | - | sequence-point | sequence-point | - / - |
| `p34c:w3` `x = (x += 1); return x;` | unsequenced | - | sequence-point | sequence-point | - / - |
| `p34c:w5` `x = (c ? x++ : 0); return x;` | unsequenced | - | sequence-point | sequence-point | - / - |
| `p34c:w6` `x = x++ + 1; return x;` | unsequenced | - | sequence-point | sequence-point | - / - |
| `p34c:w7` `x += (x = 1); return x;` | unsequenced | - | sequence-point | - | - / - |
| `p34c:w8` `x = (1, x++); return x;` | unused-value,unsequenced | unused-value | unused-value,sequence-point | unused-value,sequence-point | 174 / 174 |
| `p34c:w9` `x = c && x++; return x;` | unsequenced | - | sequence-point | sequence-point | - / - |
| `p34c:w11` `x = x + (x = 1); return x;` | unsequenced | unsequenced | sequence-point | sequence-point | - / - |
| `p34c:w12` `int y; y = x++ + (x = 1); return x + y;` | unsequenced | unsequenced | sequence-point | sequence-point | - / - |
| `p34c:w15` `return g(x = 1) + g(x = 2);` | unsequenced | unsequenced | sequence-point | sequence-point | - / - |
| `p34c:w21` `int y = (x++ , 0) + (x++ , 0); return x + y;` | unsequenced | unsequenced | - | - | - / - |
| `p34c:w16` `return (x = 1) ? x : 0;` | - | - | - | - | 187 / 187 |
| `p34d:m2` `(void)(a, b); return b;` | unused-value | unused-value | unused-value | unused-value | 174 / 174 |
| `p34d:m4` `return (x++, (void)0, 3) + x;` | unsequenced | unsequenced | - | sequence-point | - / - |
| `p34d:m10` `x = 1, y; return x + y;` | unused-value | unused-value | unused-value | unused-value | - / - |
| `p34d:m12` `(void)g(a), 1; return x;` | unused-value | unused-value | unused-value | unused-value | - / - |
| `p34d:m14` `x++ == 1; return x;` | unused-comparison | unused-comparison | unused-value | unused-value | - / - |
| `p34d:m15` `!x++; return x;` | unused-value | unused-value | unused-value | unused-value | - / - |
| `p34d:m21` `(x = 1) + 0; return x;` | unused-value | unused-value | - | unused-value | - / - |
| `p34d:m22` `(x = 1) == y; return x;` | unused-comparison | unused-comparison | unused-value | unused-value | - / - |
| `p34d:m23` `x++ * 2; return x;` | unused-value | unused-value | unused-value | unused-value | - / - |
| `p34d:m24` `(x += 1) + 1; return x;` | unused-value | unused-value | unused-value | unused-value | - / - |
| `p34d:m17` `a && (x = 1); return x;` | - | - | - | - | 187 / 187 |
| `p34d:m18` `a \|\| (x = 1); return x;` | - | - | - | - | 187 / 187 |


Silent in all four configurations and in EDG (115 shapes): `p33:a1` `int y = (x = a) + 1; return y + x;`; `p33:a2` `y = x = a; return y + x;`; `p33:a3` `short s; int i; i = s = v; return i + s;`; `p33:a4` `int y = (x += 2) * 3; return y + x;`; `p33:a5` `int y; y = x++; return y + x;`; `p33:a6` `int y; y = ++x; return y + x;`; `p33:a7` `int y; y = x--; return y + x;`; `p33:a8` `return x++;`; `p33:a9` `switch (x++) { case 0: return 1; default: return 2; }`; `p33:a10` `return f(x = a);`; `p33:a11` `if ((x = a - 1) > 0) return x; return 0;`; `p33:a12` `if ((x = a)) return x; return 0;`; `p33:a13` `int c = 0; while (i++ < n) c++; return c + i;`; `p33:a14` `int c = 0; do { c++; } while (i++ < n); return c + i;`; `p33:a15` `for (i = 0, j = n; i < j; i++, j--) c++; return c + i + j;`; `p33:a16` `return (x = a, x + 1);`; `p33:a17` `(void)g(), i++; return i;`; `p33:a18` `return n > 0 && (x = n) > 1;`; `p33:a19` `return c ? x++ : x--;`; `p33:a20` `bool b = false; int x = a; b = (x = 5); return b + x;`; `p33:a21` `bool b; int y = (b = true); return y + b;`; `p33:a22` `enum color c = RED; int k = (c = GREEN) == GREEN; return k + c;`; `p33:a25` `v(), x++; return x;`; `p33b:c25` `v(), v(); return a;`; `p33b:c2` `if ((x = a)) return x; return 0;`; `p33b:c3` `if (((x = a))) return x; return 0;`; `p33b:c4` `if ((x = a) != 0) return x; return 0;`; `p33b:c6` `do { a--; } while ((x = a)); return x;`; `p33b:c7` `for (; (x = a); ) { a--; } return x;`; `p33b:c8` `if (!(x = a)) return x; return 0;`; `p33b:c9` `return (x = a) && b;`; `p33b:c10` `return b && (x = a);`; `p33b:c11` `if (c ? (x = a) : 0) return x; return 0;`; `p33b:c12` `if (x = a, x > 0) return x; return 0;`; `p33b:c13` `(void)(x = a); return x;`; `p33b:c14` `(void)x++; return x;`; `p33b:c15` `x = a; ++x; --x; x--; return x;`; `p33b:c16` `short s = 1; int y = (s += 1000) + a; return y + s;`; `p33b:c17` `unsigned u = 1; int y = (u += -1) > 0; return y + (int)u + a;`; `p33b:c18` `short s = 1; int y = s++ + a; return y + s;`; `p33b:c19` `unsigned char c = 1; int y = c++ + a; return y + c;`; `p33b:c20` `return x++;`; `p33b:c21` `long l; int i = a; l = i = 5; return (int)l + i;`; `p33b:c22` `int y = (x = a) ? 1 : 2; return y + x;`; `p33b:c23` `int y = 1; v(), x = (y = 2); return x + y;`; `p33b:c26` `int y = (x = 1, x = 2); return x + y;`; `p33b:c27` `return (x++, x++);`; `p33b:c28` `int y = (v(), x); return y;`; `p33b:c29` `x = f(x = a); return x;`; `p33b:c30` `int y = (x = a), z = x; return y + z;`; `p33b:c31` `++x; return x;`; `p33b:c32` `return x ? ++x : --x;`; `p33b:c33` `return c ? (x = 1) : (y = 2);`; `p33b:c34` `int *p = &x; (*p)++; return x;`; `p34:k6` `return (x++, 1);`; `p34:h14` `if (x++) return x; return 0;`; `p34:u14` `return x = x + 1;`; `p34:u15` `return (x = 1, x) + 1;`; `p34:u18` `return x++ && x;`; `p34:u19` `return x++ , x;`; `p34:k5` `return y;`; `p34:k7` `(void)a, x++; return x;`; `p34:k12` `v(), v(); return x;`; `p34:k13` `g(a), x++; return x;`; `p34:k14` `return (void)0, x;`; `p34:h3` `if (x += a) return x; return 0;`; `p34:h6` `if (!(x = a)) return x; return 0;`; `p34:h7` `return (x = a) && b;`; `p34:h8` `return b && (x = a);`; `p34:h9` `return (x = a) ? 1 : 2;`; `p34:h10` `if ((x = a)) return x; return 0;`; `p34:h11` `if ((x = a) != 0) return x; return 0;`; `p34:h12` `if (x = a, x) return x; return 0;`; `p34:h15` `if (x -= a) return x; return 0;`; `p34:h16` `return (x = a) \|\| a;`; `p34:h17` `while ((x = a)) { a--; } return x;`; `p34:h18` `if ((x = a) == 0) return x; return 0;`; `p34:h19` `if (a && (x = a)) return x; return 0;`; `p34:h20` `if (a ? (x = a) : 0) return x; return 0;`; `p34:h21` `if (a \|\| x++) return x; return 0;`; `p34:h22` `x = (x = a) ? 1 : 2; return x;`; `p34:d5` `(void)(x++ + 1); return x;`; `p34:d11` `a ? x++ : 0; return x;`; `p34:d15` `(x++); return x;`; `p34b:j6` `if (x += 1) return x; return a;`; `p34b:j19` `if (a == (b = 3)) return x + b; return a;`; `p34c:w4` `x = (x++, 2); return x;`; `p34c:w10` `int y = (x = 2, x++); return x + y;`; `p34c:w13` `int y = x = x + 1; return x + y;`; `p34c:w14` `return x = g(x++);`; `p34c:w17` `return (x++ , x++);`; `p34c:w18` `return x++ ? x : 0;`; `p34c:w19` `int y = x++, z = x; return y + z;`; `p34c:w20` `int y = (x = 1) + (a = 2); return x + y + a;`; `p34c:w22` `return (c ? x++ : x--) + 1;`; `p34c:w23` `return c ? x++ : (x = 2);`; `p34c:w24` `return (x = 3) + a;`; `p34d:m25` `(void)(x += 1) + 1; return x;`; `p34d:m1` `x++, (void)y; return x + y;`; `p34d:m3` `(void)(x++, y); return x + y;`; `p34d:m5` `for (x = 0, y = 1; x < 3; x++, y++) { a++; } return x + y + a;`; `p34d:m6` `y = (x++, y); return x + y;`; `p34d:m7` `x++, g(y); return x + y;`; `p34d:m8` `x++, x++; return x;`; `p34d:m9` `(x = 1, y = 2); return x + y;`; `p34d:m11` `g(a), (void)0; return x;`; `p34d:m13` `(void)(x++ + 1); return x;`; `p34d:m16` `(void)x; return x;`; `p34d:m19` `a ? (x = 1) : (x = 2); return x;`; `p34d:m20` `a ? x++ : x--; return x;`; `p34d:m26` `return (void)(x = 1), x;`; `p34d:m27` `y = (x = 1, x++, 3); return x + y;`; `p34d:m28` `(x = 1); return x;`; `p34d:m29` `(x++); return x;`; `p34d:m30` `((void)0, x++); return x;`

### S17: the top-test loop shape: **PASS (every twin fails at its line; Silicon never threw)**

Hand-written Viper in the style of the emitted code (`spikes/do_count.vpr`, `continue_skip.vpr`, `uninit_nested_loops.vpr` are `build/cerastes verify --keep-vpr` output of existing tests); `spikes/gen17.py` writes `spikes/s17*.vpr` and `.out`.
The shape, for C `while (i++ < n) { c++; }` (`spikes/s17a_ok.vpr`): the automatic range invariants of the written locals, the user invariant, its well-definedness conjunct, `decreases`, and a body that holds the condition's effects and test:

```text
  while (true)
    invariant (-2147483648) <= v_i && v_i <= 2147483647          // automatic range invariants (i, c)
    invariant (-2147483648) <= v_c && v_c <= 2147483647
    invariant v_i == v_c && 0 <= v_c && v_c <= v_n                // the user's invariant, at the head: before the condition
    invariant (...) ==> !((v_n - v_i) < (-2147483648) || 2147483647 < (v_n - v_i))     // its well-definedness / the measure's
    decreases v_n - v_i
  {
    t_old := v_i                                // snapshot of the read of i (E3)
    assert !((v_i + 1) < (-2147483648) || 2147483647 < (v_i + 1))      // the condition's own obligation, at every evaluation
    v_i := v_i + 1
    t_c := t_old < v_n
    if (!t_c) { goto l_brk }
    assert ...; v_c := v_c + 1                  // the body
    label l_cont                                // `continue` target; a `for` step goes here
  }
  label l_brk
```

| file | change | Silicon result (`.out`) |
|---|---|---|
| `s17a_ok` | none | verified |
| `s17a_entry` | `v_i := 1` before the loop | `Loop invariant ... might not hold on entry. Assertion v_i == v_c might not hold. (@21.15--21.51)`: the invariant's line |
| `s17a_preserved` | body adds 2 to `c` | `Loop invariant ... might not be preserved. Assertion v_i == v_c might not hold. (@21)` |
| `s17a_term` | `decreases v_n` | `Loop might not terminate. Termination measure might not decrease or might not be bounded. (@23.15--23.18)`: the `decreases` clause |
| `s17a_termunbounded` | `decreases 0 - v_i` (decreasing but not bounded) | the same message at the `decreases` clause |
| `s17a_termnonneg` | `decreases v_n - v_i - 1` (negative at the last head) | **verified**: the bound is checked on the back edge only (a head from which the test fails and the loop exits needs no bound) |
| `s17a_lastoverflow` | `requires n <= 2147483647` | `Assert might fail. Assertion !(v_i + 1 < -2147483648 || ...) might not hold. (@27.12--27.66)`: the `i++` assertion in the condition, failing only when `i == INT_MAX` at the last evaluation |
| `s17a_post` | invariant without `c <= n` | two errors: the same overflow assertion (@27) and `Assert might fail. Assertion res == p_n might not hold. (@39)`: the exit state is needed |
| `s17a_post_exitstate` | `res == p_n + 1` | `Assert might fail. Assertion res == p_n + 1 might not hold. (@39.10--39.24)` |
| `s17b_continue_ok` | `while (i++ < n) { if (i <= 2) continue; c++; }` with `goto l_cont` | verified |
| `s17b_continue_bad` | invariant `i > 3` instead of `i > 2` | `Loop invariant ... might not be preserved. (@21.15--21.96)` |
| `s17b_break_ok` | `while (i++ < n) { if (i == 5) break; c++; }` with `goto l_brk` from the body | verified (`res == min(n, 4)`) |
| `s17b_break_bad` | postcondition `res == 5` on the break path | `Assert might fail. Assertion ... res == 5 might not hold. (@41.10--41.59)` |
| `s17c_flag_ok` | `int x; int c = 0; x = 0; while (c < n && (x = c + 1) > 0) c++; return x;`: `d_v_x`, `ds_v_x := d_v_x` before the loop, `invariant ds_v_x ==> d_v_x`, the branch form for `&&` | verified |
| `s17c_flag_nodefine` | no `x = 0` before the loop | `Assert might fail. Assertion d_v_x might not hold. (@44.10--44.15)`: the read after the loop (the `&&` writes `x` in a branch only) |
| `s17c_flag_noDS` | `invariant ds_v_x ==> d_v_x` dropped | `Assert might fail. Assertion d_v_x might not hold. (@45)`: the `ds_` snapshot invariant is what carries the flag across the havoc |
| `s17c_range_dropped` | the automatic range invariant of `x` dropped | `Postcondition of ... might not hold. Assertion -2147483648 <= res might not hold` and `res <= 2147483647`: the range of a local written only in a branch of the condition is needed after the loop |
| `s17d_condonly_ok`, `_norange` | `int x = 0; while ((x = c + 1) <= n) c++;`, user invariant mentioning `x - 1` | both **verified**: when the condition always writes `x` first, the automatic range invariant is not needed (the assertion before the write gives the range); it matters when the write is under `&&`/`?:` |

Silicon error ids for the harness (`scripts/run_tests.py` patterns): invariant entry `invariant.not.established`, preservation `invariant.not.preserved`, `Assert might fail` `assert.failed`, `Loop might not terminate` `termination.failed`,
postcondition `postcondition.violated`. The `.vpr` of the five verifying spikes passes `scripts/lint_viper.py` once a position map exists (`spikes/mkmap.py` writes a dummy one; `lint_viper: 5 files clean`): the shape has no backward
`goto`, no label with invariants, `goto l_brk` leaves the loop outward, `goto l_cont` stays in the loop body's block; `d_v := true` follows `v := ...` and `d_v := false` precedes `v := 0`, as the planned flag rule requires. The position map is
per `.vpr` line and id, so the condition's asserts (one per line, inside the loop body) map as every other check does; no ambiguity was seen.

### S18: branch lowering and read snapshots: **PASS (every twin fails at its line)**

`spikes/gen18.py` writes `spikes/s18*.vpr` and `.out`. The prelude has `c_div` as emitted and trusted methods `f_abort__9` (`ensures false`, `decreases _`) and `f_g__8`.

| file | Viper (hand-lowered) | result |
|---|---|---|
| `s18a_and_ok` | `y = n > 0 && (x = n) > 1;`: `t := v_n > 0; if (t) { v_x := v_n; d_v_x := true; t := v_n > 1 }; v_y := t ? 1 : 0` (E4: the value of `(x = n)` is `v_n`) | verified, `res == (p_n > 1 ? 1 : 0)` |
| `s18a_and_badvalue` | postcondition `> 2` | `Assert might fail. Assertion res == (p_n > 2 ? 1 : 0) might not hold. (@37)` |
| `s18a_and_readx_untaken` | a later read of `x` (`assert d_v_x`), no precondition | `Assert might fail. Assertion d_v_x might not hold. (@35.10--35.15)`: fails on the path that skips the branch only |
| `s18a_and_readx_taken` | the same with `requires p_n > 0` | verified (only the taken path is feasible) |
| `s18a_and_readx_wrongreq` | the same with `requires p_n < 0` | the flag assertion fails |
| `s18a_and_value_via_vx` | `t := v_x > 1` (value read back from `v_x`) | verified: harmless right after the write, but E4 does not rely on it |
| `s18a2_guarded_div_ok` / `_bad` | `n > 0 && 100 / (x = n) > 1`: the division assertion `v_n != 0` inside the branch; guard `n > 0` / `n >= 0` | verified / `Assert might fail. Assertion v_n != 0 might not hold. (@32.12--32.20)` (at `n == 0`) |
| `s18b_cond_ok` | `y = c ? (x = 1) : (x = 2); return x + y;`: `if (v_c != 0) { v_x := 1; d := true; t := 1 } else { v_x := 2; d := true; t := 2 }` | verified, `res == (c != 0 ? 2 : 4)` |
| `s18b_cond_onearm` / `_noread` | the else arm does not write `x`; with / without the later read | the flag assertion fails (`@37.10--37.15`) / verified |
| `s18b_cond_badvalue` | wrong postcondition | `Assert might fail. Assertion res == (p_c != 0 ? 2 : 3) might not hold. (@42)` |
| `s18c_snap_ok` | `r = (y = x, x = 1, y + x)`: `t_x1 := v_x; assert d_v_x; v_y := t_x1; v_x := 1; d_v_x := true; t_y2 := v_y; t_x2 := v_x; assert no-overflow(t_y2 + t_x2); v_r := t_y2 + t_x2` | verified, `res == p_x + 1` with `p_x <= 2147483646` |
| `s18c_snap_overflow` | no precondition | `Assert might fail. Assertion !(t_y2 + t_x2 < -2147483648 || ...) might not hold. (@38.10--38.72)` (at `x == INT_MAX`) |
| `s18c_snap_uninit` | `x` has no value before | `Assert might fail. Assertion d_v_x might not hold. (@32.10--32.15)`: the `uninitialized` obligation at the snapshot's slot |
| `s18c_snap_wronglowering` | the snapshot of `x` for `y = x` taken after `x := 1` | `Assert might fail. Assertion res == p_x + 1 might not hold. (@42)`: why E3 needs the snapshot before the write |
| `s18d_noreturn_ok` | `(abort(), 1 / d)`: `f_abort__9(); assert v_d != 0; assert !(1 == INT_MIN && v_d == -1); res := c_div(1, v_d)` | **verified with `d` unconstrained** (the checks follow the call that ends the program) |
| `s18d_noreturn_swapped` | checks before the call | `Assert might fail. Assertion v_d != 0 might not hold. (@23.10--23.18)` |
| `s18d_returning_call` | `g()` that returns | `Assert might fail. Assertion v_d != 0 might not hold. (@24.10--24.18)` |

Mapping notes: every assertion sits on its own line, including those inside the `if (t) { ... }` branch and the snapshot's flag assertion, so line + id stays unique per check. Two checks of one full expression that Silicon reports
at the same `.vpr` line cannot occur in these shapes. The only surprise is `s17a_termnonneg` (a measure that is negative at the exiting head still verifies), a property of Silicon's back-edge check, not of the shape.

## M4.3b: calls in expressions

Probe P36 and spikes S19-S20 for decision 0021. Evidence is under `build/m4.3b/probes/` and `build/m4.3b/spikes/` (gitignored). Silicon is `6b540aa4` (`Silicon 1.1-SNAPSHOT (6b540aa4@(detached))`, silver as pinned), Z3 4.16.0, run as `build/m4.3b/spikes/s19lib.py` does (`$CERASTES_JAVA -Xss128m -jar $SILICON_JAR --z3Exe $Z3_EXE --numberOfErrorsToReport 0 <file.vpr>`; the output of each run is the `.out` beside its `.vpr`). `il_def.h` lines are in `third_party/edg/src/il_def.h` at the pin `f0ed5525af`.
P36's IL dumps ran as `build/m4.3b/fe.sh <probe> c23|c++20` (`scripts/cerastes-fe.sh --display_error_number --il_display`); `build/m4.3b/tree4.py <il> fn` condenses a function body to a tree like M4.3a's `tree3.py` and adds, with `POS=1`, `plain` or `MACRO(orig seq:col)` per node (see 3 below). `build/m4.3b/matrix.py` ran Apple clang (clang-2100.3.34.2) and GCC 16.2.0 with `-fsyntax-only -Wall -Wextra`, C23 and C++20 (`matrix.txt`, `matrix.json`, `matrix_detail.json`; the list below comes from `table36.py`). Probe files, one function per line: `p36.c|cpp` (`a1`-`a40`, plus a template in the `.cpp`), `p36b.c|cpp` (`b1`-`b18`, macros), `p36c.c|cpp` (`c1`-`c7`, macro and layout extras). The `.cpp` files are copies (`p36.cpp` adds `t1`, `t2`).

### P36: `-Wself-assign`: **PASS (no stop rule triggered)**

**1. Who warns.** GCC 16 has no such warning: it is silent on every function of the three probes with `-Wall -Wextra` in both modes. Apple clang warns with `-Wself-assign`, text `explicitly assigning value of variable of type 'int' to itself` (`'enum color'`, `'T'` for the other types; `p36:a14` in C++ prints it twice), in both modes alike (`matrix_detail.json`). EDG gives **no** diagnostic of its own for any shape: the plain front end in both modes prints only #550 (`parameter "y" was set but never used`, lines of `a5`, `a18`, `a29`: an artefact of the probe) and #174 (`expression has no effect`, `a28`, `x = (x, x);`, a shape cerastes already rejects as `comma_without_effect`); `build/cerastes verify p36.c` shows the same warnings, then cerastes's own errors (`probes/p36.c.c23.err`, `p36.cpp.c++20.err`, `p36b`/`p36c`: nothing). So no EDG warning is on a shape that cerastes accepts and clang rejects, and the verify-mode warnings are unchanged.

**2. The verdict by shape** (clang C = clang C++ unless stated; GCC and EDG: no warning for any row).

Warns (`-Wself-assign`):

| shape | probe |
|---|---|
| `x = x;`, `x = (x);`, `(x) = x;`, `(x) = (x);`, `x = ((x));` | a1-a4, a27 |
| `y = (x = x);` (the inner assignment), `y = x = x;` (the inner), `x = (x = x);` (the inner; also `-Wunsequenced` in C only) | a5, a29, a39 |
| the `for` step `x = x`; `return (x = x, y);`; `x = x, y++;` (discarded comma); `if ((x = x))`; `while ((x = x))`; `(void)(x = x);`; `int y = (x = x) + 1;` | a6, a7, a8, a9, a40, a31, a30 |
| `short s; s = s;`, `bool b; b = b;`, `enum color e; e = e;`, a local `int x = 1; x = x;`, a `do { x = x; } while (0);` body, a global `gl = gl;` | a11, a13, a14, a15, a24, a35 |
| the statement over two lines (`x =\n x;`), an earlier `x = x;` followed by a macro statement, `#line` | b15, b14/b18, c6 |
| parentheses written by a macro around a written operand: `#define LP (` then `x = LP x RP;`; a comment: `x = (x) /* c */;` | c1, c5 |
| a template definition `template <class T> void t1(T x) { x = x; }` (`'T'`): **C++ only, and only at the definition, none at the instance** (cerastes rejects every template at M4, `unsupported_declaration`) | p36.cpp line 45 |

Silent (all four configurations):

| shape | probe |
|---|---|
| an explicit cast on the right: `x = (int)x;`, `x = (int)(x);`, `s = (short)s;`, `long x; x = (int)x;`, `unsigned x = (unsigned)x` | a10, a32, a12, a33, a34 |
| any other right operand: `x = +x;`, `x = x + 0;`, `x = x ? x : x;`, `x = y = x;` (the outer `x = <assignment>`), `x = (x, x);` (clang `-Wunused-value` only), compound assignments `x += x;`, `x -= x;` | a16, a17, a36, a18, a28, a25, a26 |
| `sizeof(x = x)` (clang `-Wunevaluated-expression` only) | a23 |
| **any macro involvement** in the operator or either operand: `#define SET(a, b) a = b` with `SET(x, x)`, `SET(x, (x))`, `SET((x), x)`; `#define X x` with `X = x`, `x = X`, `X = X`; `ID(x) = x`, `x = ID(x)`, `ID(x) = ID(x)`; `x EQ x` (`#define EQ =`); `SELF(x)` (`#define SELF(v) (v = v)`); `E;` (`#define E x = x`); `x = PAREN(x)`; `SETX` (a `do { x = x; } while (0)` macro); `WRAP(x = x;)` (the whole statement is a macro argument); `#define BODY { x = x; }`; `PAIR(x, ID(x))`; multi-line `PAIR(x,\n x)`; `WRAP(x) = x`; `x = WRAP(x)`; `x = (WRAP(x))`; `#define L x =` with `L x;`; `#define R = x` with `x R;`; `x = LP RX RP;` (`#define RX x`) | a19-a22, a37, a38, b1-b13, b16, b17, c2, c3, c7 |

So clang's rule is: both operands, after dropping parentheses and implicit conversions, name the **same variable**, the right one is not under an explicit cast or any other operator, **and neither operand nor the `=` token comes from a macro expansion** (including a macro argument: `ID(x) = x` is silent) and the expression is not in an unevaluated context. A parenthesis written by a macro does not matter (c1 warns: the `x` is a written token).

**3. IL facts** (`probes/p36.c.c23.trees`, `p36.cpp.c++20.trees`, `p36b.*.trees`, `p36c.*.trees`; one line per function; `@seq:column` is `position`).

(a) *The shape of the node.* An assignment is `eok_assign` (`il_def.h:14120`) with two operands. The left operand is an `enk_variable` with `is_lvalue`; the right is the `enk_variable` with `orig_lvalue_type` (the value read, `orig_lvalue_type` at `il_def.h:14917`: the lvalue-to-rvalue conversion clears `is_lvalue`, no node), directly, with no generated cast, for every same-type shape, in both modes (`a1`, `a11` short, `a13` bool, `a14` enum, `a15`):

```text
a1  stmk_expr eok_assign[int] {nu} @5:20 plain(var x:int {lv} @5:18 plain, var x:int {olt=int} @5:22 plain)                 (C23)
a1  stmk_expr eok_assign[int] {lv,nu} @5:20 plain(var x:int {lv} @5:18 plain, var x:int {olt=int} @5:22 plain)            (C++20)
a11 eok_assign[short] {nu} @15:23 plain(var s:short {lv} @15:21 plain, var s:short {olt=short} @15:25 plain)
```
No node is generated between the variables in any warned shape, so no compiler-generated cast needs peeling: a peeling rule (drop `compiler_generated` casts only) would be harmless. An explicit cast is its own `eok_cast` **without** `compiler_generated` (`a10`: `eok_cast[int] @14:23 plain(var x...)`; `a12`; `a33`: `eok_cast[long] {gen}(eok_cast[int](var x:long))`: the generated one is peeled, the explicit one stops), which is clang's silent shape. `+x`, `x + 0`, `?:`, `,` and the compound assignments are other node kinds (`eok_unary_plus`, `eok_add`, `eok_question`, `eok_comma`, `eok_add_assign`), silent.

(b) *Parentheses.* No node: `is_parenthesized` (`il_def.h:15075`) is on the operand or the assignment (`a2` the right `x {par}`; `a3` the left `{lv,par}`; `a4` both; `a5` the inner assignment `{par}`; `a27` `{par}`). The flag does not matter to clang's rule, so the detector ignores it.

(c) *Macro origin.* `RECORD_MACROS_IN_IL=1` records only the `#define`s (`iek_macro`); what tells a macro-expanded node apart is the source position (`a_source_position`, `basics.h:896-923`, `FULLY_RESOLVED_MACRO_POSITIONS`): `seq`/`column` give the start of the **top-level macro invocation**, `orig_seq`/`orig_column` the origin of the token (the macro definition, or the argument text of the invocation); outside a macro `orig == (seq, column)`, and `il_display` prints `orig_*` only when they differ (`il_display.c:587`). On a node: `position` (`il_def.h:15110`; the operator for an operation node, the token for a variable). Observed, `tree4.py`'s `plain` (orig equal) or `MACRO` (orig different):

```text
a1  eok_assign @5:20 plain(var x @5:18 plain, var x @5:22 plain)                                          written: all three plain
a19 eok_assign @23:19 MACRO(orig 1:21)(var x @23:19 MACRO(orig 23:23), var x @23:19 MACRO(orig 23:26))   SET(x, x): the = in the #define (line 1), the operands are the argument text
a20 eok_assign @24:21 plain(var x @24:19 MACRO(orig 2:11), var x @24:23 plain)                            X = x: only the left token is a macro
a21 eok_assign @25:21 plain(var x @25:19 plain, var x @25:23 MACRO(orig 2:11))                            x = X
b1  eok_assign @10:24 plain(var x @10:18 MACRO(orig 10:21), var x @10:26 plain)                           ID(x) = x: an argument token (orig = the argument's column)
c1  eok_assign @7:20 plain(var x @7:18 plain, var x {par} @7:25 plain)                                   x = LP x RP: written tokens, the parentheses are not in the IL
c2  eok_assign @8:18 MACRO(orig 3:13)(var x @8:18 MACRO(orig 3:11), var x @8:20 plain)                    L x; with L = `x =`
b9  eok_assign @18:18 MACRO(orig 18:25)(var x @18:18 MACRO(orig 18:23), var x @18:18 MACRO(orig 18:27))  WRAP(x = x;): a macro argument (orig is the user's text; seq/column the invocation)
c6  eok_assign @14:5 plain(var x @14:3 plain, var x @14:7 plain)                                          after `#line 100`: plain, as clang warns
```
The rule "no operand and no operator token is from a macro" is therefore decidable: **`position.orig_seq == position.seq && position.orig_column == position.column`** for the assignment node, for the left variable node and for the right variable node. It reproduces clang's verdict on every probe: `build/m4.3b/detector36.py` applies the rule to the four sets of `.trees` (C23 and C++20 for `p36`, `p36b`, `p36c`) and compares with `matrix.json`: **132 (function, mode) pairs, 0 mismatches**, the detector firing on 57 (`b15`, `c6` and the template `t1` are keyed by line in `matrix.json`; the detector fires on all three, and clang warns at their lines or at the template definition). The silent macro shapes (b1-b13, b16, b17, c2, c3, c7, a19-a22, a37, a38) each have at least one `MACRO` node; the written-but-silent shapes (a10, a12, a16, a17, a18, a23, a25, a26, a28, a32-a34, a36) are separated by the node kinds, not by positions. The existing `macro_node` test in `encode.cpp` (`e->position.orig_seq != 0`) is **not** this rule (for a plain node `orig_seq == seq != 0`): the new detector compares the pairs.

**Proposed detector** (for the `self_assignment` rule, after M4.3a's detectors): at every `eok_assign` node reached in code (a statement, a used value, a `for` step, a condition, a `return`): (1) strip `compiler_generated` `eok_cast`/`eok_bool_cast` wrappers from both operands (never an explicit cast); (2) both results are `enk_variable` and name the same variable; (3) the assignment node's position and each of the two variable nodes' positions have `orig_seq == seq && orig_column == column`; then reject, reported at the assignment (the `=`). A compound assignment, `++`/`--` and anything inside `sizeof`/`typeof` (already rejected by 0020) are not self-assignments. The shapes where cerastes would reject but clang is silent under `-Wall -Wextra` are none (the rule is the same set); the shapes clang warns and cerastes already rejects for another reason are `a35` (a global, 0012), `a28`-like commas (`comma_without_effect`), `a39` (`unsequenced_modification`, which wins by reporting first or together) and the template definition (`unsupported_declaration`).

**Verdict: PASS.** Clang's verdict turns only on facts the IL keeps (same variable, no node between, explicit casts as separate nodes, macro origin through `orig_*`). Parentheses are irrelevant to clang. No EDG warning exists for a shape cerastes accepts. Stop rule 3 does not fire.

### S19: the termination check placed by cerastes (decision 0021, option B): **PASS (the explicit assert is equivalent to Silicon's own check, including unequal lengths)**

**1. Silicon's rule, from the source** (`third_party/silicon/silver/src/main/scala/viper/silver/plugin/standard/termination/transformation/`). A call to a method of the **same strongly connected component** as the caller (`MethodCheck.scala:66`; the sets come from Kosaraju over a call graph that has an edge for **every `MethodCall` node syntactically in a method body**, `:268-285`, so dead code counts and a self-call is its own component) gets, immediately before it, `assert (caller condition ==> lex(callee tuple over the call's arguments, old(caller tuple)))` (`createTupleCheck`); `createLexDecreaseCheck` (`DecreasesCheck.scala:111-165`, `createExp`) is, with `s` the callee's tuple over the arguments and `b` the caller's, "both tuples extended with a TOP element":

```text
createExp(b, s) = false                                          if s is empty         (:130)
                  true                                           if s is not empty and b is empty   (:134)
                  (decreasing(s0, b0) && bounded(b0)) || (s0 == b0 && createExp(b', s'))   otherwise
```
with `decreasing(x, y) <== x < y` and `bounded(x) <== x >= 0` for `Int` (`silver/src/main/resources/import/decreases/int.vpr`, the axioms of `IntWellFoundedOrder`; implications, so Silicon proves exactly what it can derive from `x < y`, `x >= 0`). Unfolded, with N = the callee's measure over the arguments (`s`), M = the caller's over its formals (`b`; formals are immutable, so `old` is the entry value): **`⋁ᵢ<min(|N|,|M|) (⋀ₗ<ᵢ Nₗ == Mₗ ∧ Nᵢ < Mᵢ ∧ Mᵢ ≥ 0)`, and when `|N| > |M|` also `⋀ₗ<|M| Nₗ == Mₗ`** (a longer tuple that agrees on the whole shorter one is smaller; equal tuples, and a shorter `N` that agrees on its length, fail). For equal lengths this is exactly 0013 item 2's formula, so plan fallback A (immovable calls) is **not needed** for unequal lengths either. A call to a method **outside** the component gets only `assert callee.terminationCondition` (`:111-125`), which is `true` for a callee with a `decreases` tuple without condition: no decrease check (case 8).

**2. The matrix.** `build/m4.3b/spikes/s19a.py`, `s19b.py`, `s19c.py` write, per point, V0 (the call alone) and V1 (the same method with `assert lex(N, M)` on one line right before the call: `s19lib.lex`, the De Morgan form `!(!alt1 && !alt2 …)`; see the note after the table) in the style of the emitted Viper (`spikes/base/{recursion,measure_mutual,noreturn_recursive}.vpr` are the real emitted files they copy: `requires` range per formal, `ensures` range of `res`, `decreases` the measure over `p…`, locals `v…` copied from the formals, `assert noov(arg)` per argument, `t := f(args)`), runs Silicon once per file (many independent methods per file, errors mapped back by line) and compares: **V1's `assert` fails exactly when V0's termination check fails**, point by point. The state of a point is fixed by an `inhale p == c` (spike only). Results (each `s19_case*.txt` lists every point; `mismatches` 0 everywhere; "own-term" = Silicon's own check reporting in V1 as well):

| case | shapes | points | V0 fails / passes | mismatches | V1 own-term reports |
|---|---|---|---|---|---|
| 1 one component | `n-1`, `n`, `n+1`, `0`, `-1`, `n-2` at n = -1, 0, 1, 3 (old value -1; `n-1` at n = 0 passes, `n` and `n+1` fail) | 24 | 13 / 11 | 0 | 0 |
| 2 two components | `(a∓1/a/a+1, b+100/b-1/b/0/-5/b+1)` at a, b in {-1, 0, 1}; includes `(a-1, b+100)`, `(a, b-1)` at b = 0, `(a, b)`, `(a+1, b-1)`, `(a-1, -5)` | 162 | 111 / 51 | 0 | 0 |
| 3 three components | all 27 argument shapes at 12 states (`c` = -1, 0, 1); `3b`: equal up to the last, negative middles | 324 + 42 | 172 / 152 and 18 / 24 | 0 | 0 |
| 4 mutual, equal lengths | `f(a,b)` calls `g(c,d)` (g's measure `(q0,q1)`, `(q1,q0)` and `(q0+q1, q0)`), 15 argument shapes at 9 states, g's call back is dead code `if (false)` (the cycle is syntactic) | 810 | 474 / 336 | 0 | 0 |
| 5 mutual, **unequal lengths** | 2→1, 1→2, 3→1, 1→3, 3→2, 2→3, up to 300 sampled argument/state points each (first, 248 V0-only points were used to read the rule off; then the source, `:129-148`, gave the exact form) | 1380 | 662 / 718 | 0 | 0 |
| 6 `c_div` measure | measure `c_div(n, 2)` (prelude function copied from `src/viper/prelude.cpp:36`); arguments `c_div(n,2)`, `n-1`, `c_div(n,2)-1`, `c_div(n,3)`, `c_div(n-1,2)`, `c_div(n,-2)`; 10 concrete states and 4 symbolic ranges | 84 | 50 / 34 | 0 | 0 |
| 7 parameter changed | `v0 := v0 + 5; f(v0 - 1)` (measure over the **entry** `p0`: must fail at every state; arguments `v0-6`, `v0-5`, `v0` too) and `v0 := v0 - 5; f(v0 + 4/+5/+6/-1)` | 48 | 33 / 15 | 0 | 0 |
| symbolic states | ranges for 1 and 2 components (the solver, not constants) | 76 | 43 / 33 | 0 | 0 |

2950 points, every one with both outcomes represented in each family. Case 7's must-fail shape: `n := n + 5; f(n - 1)` with measure `n` is reported by V0 at the call and by V1 at the assert in every state tried (`s19_case7.txt`: `OK c1 vals=['p0 == -3'] args=['v0 - 1']  V0 term-fails=1  V1 assert-fails=1`, likewise for p0 = 0, 1, 3, 8 and `1 <= p0 <= 1000`).

**3. Case 8: a non-recursive callee with a measure placed after another call.** `spikes/s19_case8.vpr`: `h1`/`h2`/`h3` have `decreases p0`, call `ret(0)` (returns) and then `g_meas(p0 + 1000)` (measure `q0`), `g_empty(p0 + 1000)` (empty `decreases`) or `g_meas(-5)`; `g` does not call `h`. **`Silicon finished verification successfully`** (`s19_case8.out`): no check at all, as `MethodCheck.scala:111-125` says, so no explicit assert is needed for a call outside the caller's component (the cerastes cycle must equal Viper's components: the VIR call graph with an edge for every call node of a body, dead code included).

**4. Case 9: the hole, and the placement.** `spikes/s19_case9_*.vpr` (`never` is the trusted `ensures false decreases _` shape of `f_abort`; the caller `f(n)` has `decreases p0` and `inhale 0 <= p0 <= 100`); outputs in the `.out` files:

| variant | Silicon | |
|---|---|---|
| `t := never(0); t2 := f(v0 + 1)` (Silicon's check alone) | `verified`, 0 errors | **the hole** (`s19_case9_alone.out`: `Silicon finished verification successfully`) |
| `t := ret(0); t2 := f(v0 + 1)` (a callee that returns) | 1 error: `Method might not terminate. Termination measure might not decrease or might not be bounded.` at the call, line 28 (`termination.failed` / `tuple.false`) | control (`s19_case9_control.out`) |
| `assert lex; t := never(0); t2 := f(v0 + 1)` | 1 error: `Assert might fail. Assertion !!(v0 + 1 < p0 && p0 >= 0) might not hold.` at the assert, line 26 (`assert.failed` / `assertion.false`) | the explicit assert **before** `never` closes the hole (`s19_case9_assert_before_never.out`) |
| `t := never(0); assert lex; t2 := f(v0 + 1)` | `verified` | placed **after** `never` it is hidden too (`s19_case9_assert_after_never.out`): the placement is the point |
| `assert lex; t := ret(0); t2 := f(v0 + 1)` | 1 error at the assert (line 26) | an intervening returning call: the failed assert is reported once, Silicon's own check at the call is then silent |
| `assert lex(n-1); t := ret(0); t2 := f(v0 - 1)` at `1 <= p0` | `verified` | `s19_case9_assert_before_ret_ok.out` |

(For one component the assert is `!(!(((v0 + 1)) < (p0) && (p0) >= 0))`, the De Morgan form of 5 below; Silicon prints it as `!!(…)`. The case 9 files were rerun with it, same verdicts.)

**5. Case 10: reports.** One report per failure in every point of the matrix, with the De Morgan form (the plain `alt1 || alt2` form gave two reports at one of the 2950 points, a `c_div` measure whose first conjunct and `M >= 0` both failed: Silicon reports each conjunct of an asserted `&&` that sits at the top or inside an alternative, so the emitted condition is `!(!alt1 && !alt2 …)`, one atomic assertion like `in_range_atomic`; the whole matrix was rerun with it, 2950 points, 0 mismatches, `V1 assert-fails` <= 1 and = `V0 term-fails` at every point):
- V0 (Silicon's own check): `Method might not terminate. Termination measure might not decrease or might not be bounded.` at the call; Silicon id `termination.failed`, reason `tuple.false` (`s19_case9_control.out`; `s19_case1.out` has the first 13).
- V1 (the explicit assert): `Assert might fail. Assertion <expr> might not hold.` at the assert; id `assert.failed`, reason `assertion.false`; **Silicon's own check at the call does not also report**: the failed assert is assumed afterwards (the M4.2b1 S14 behaviour) and the assumed condition is the check's own condition, so the check holds. Across the 2950 points (rerun with the atomic form) the counts are `V1 assert-fails` = `V0 term-fails` (0 or 1) and `V1 own-term-fails` = 0 in every row (`grep -c 'V1 own-term-fails=[1-9]'` is 0 in every `s19_case*.txt`).
- Consequence for the report layer: the explicit obligation is the only report for a failing call and carries category `termination` by construction (its own `Meta`), and `report.cpp:140-147` has nothing to de-duplicate; Silicon's `termination.failed`/`tuple.false` at the call remains the backstop that fires only if cerastes omits the assert (a call whose set already contains every earlier call).

**Verdict: PASS.** The explicit assert is equivalent to Silicon's check for equal lengths and, with the TOP-extended rule above, for unequal lengths; the one assumption is that the arguments' values at the assert's position equal their values at the call (a callee that returns cannot change a local; an argument depending on a call result puts that call into PRE(c)).

### S20: order, branches and loop conditions: **PASS**

`build/m4.3b/spikes/s20.py`, Viper in the shape `src/vir/lower.cpp` emits (items right after the calls they depend on; `branch_body` as an `if` holding the call; the top-test loop `while (true) { <obligations>; t := next(…); if (!(t < n)) { goto l_brk } body }` of `lower.cpp:1192-1199`; the `do` shape `…body…; <obligations>; t := next(…); if (!c) { goto l_brk }` of `:1212-1221`; `nr` is `ensures false decreases _` like `f_abort`). Result: `build/m4.3b/spikes/s20_result.txt` (`s20_{a,b,c,d}.vpr/.out`). Every variant below gave exactly the listed errors and no other:

| case | variant | errors |
|---|---|---|
| (a) `nr(k) + 100 / zero(y)` | order `zero`, `assert t1 != 0`, `nr` | **1: `assert.failed` at the division check** (`a_zero_first`) |
| | order `nr`, `zero`, `assert` | 0: verifies, **the hole** (`a_nr_first`) |
| | `zero` first with a caller precondition `y != 0` | still 1 at the check (`a_zero_first_with_pre`: zero's result is unconstrained) |
| (b) a branch with a call before the outer `nr` | `if (a > 0) { assert a > 0; t := g(a); assert t != 0; u := 100 / t }; t2 := nr(k)` | **1 at `assert t != 0`** (`b_branch_first`) |
| | the branch after `nr` | 0: **the hole** (`b_branch_after_nr`) |
| | `if (a >= 0) { assert a > 0 …` | 1 at the branch's precondition assert (`b_branch_pre`) |
| | the inner check guarded by `if (t != 0)` | 0 (`b_branch_guarded_ok`) |
| (c) `while (next(i, n) < n) { i++; }`, loop head invariants `0 <= i <= n-1`, `decreases n - i` | `next_loose` (pre `q0 < q1`) | 0: verifies, and `res == n - 1` after the loop holds, so the callee's `ensures res == q0 + 1` is known after the call (`c_ok`) |
| | `next_tight` (pre `q0 + 1 < q1`: fails only for the last evaluation `i = n - 1`) | **1: `assert.failed` at the precondition assert in the loop**, nothing else (`c_pre_last`) |
| | a wrong post (`res == n`) | 1 at the post assert (`c_post_wrong`: the exit state is known, not vacuous) |
| | body `i := i + 0` | 1 `termination.failed`/`tuple.false` at the loop's `decreases` (`c_term`) |
| | body `i := i + 3` | 1 `invariant.not.preserved` at the user invariant (`c_inv`) |
| | user invariant `1 <= i <= n-1` | 1 `invariant.not.established` at it (`c_entry`) |
| (d) `do { i++; } while (next(i, n) < n);`, invariants at the start of every iteration `0 <= i <= n-2`, `n >= 2` | `next_loose` | 0, `res == n - 1` holds (`d_ok`) |
| | `next_tight` | 1 at the precondition assert at the end of the body (`d_pre_last`) |
| | wrong post / body `i := i + 0` | 1 post assert / 1 `termination.failed` (`d_post_wrong`, `d_term`) |
| | user invariant `i <= n - 3` | 2: `invariant.not.established` and `invariant.not.preserved` at it (`d_inv`) |

The facts for the lowering: (a)/(b) are the hole 0021's chain rule closes by ordering (a call that does not return must come after every item whose check C may evaluate first, and a branch holding a call is such an item); (c)/(d) need nothing new in Viper: the call in a loop condition is an ordinary call of the body, its precondition is an ordinary assert reported where it fails, and the loop's invariants, measure and exit knowledge behave as for M4.3a's top-test loops. One expectation corrected while writing the spike: in a `do` loop the exit test comes before the invariant is checked, so `i := i + 3` is reported first at the callee's precondition, not as an invariant failure (`d_inv` therefore uses a tighter invariant).

### P37: discarded guarded calls, and a discarded `&&`/`||` with a side effect on its left: **PASS (the union rule extends; the detector corrected)**

Run by the orchestrator after audit round 1 (item 6): `build/m4.3b/probes/p37/` (`p37.c`, `p37c.c` with Apple clang and
GCC 16, `-Wall -Wextra -fsyntax-only`, `-std=c23` and `-x c++ -std=c++20`; `p37b.c`, `p37d.c` the same shapes with
contracts through `build/cerastes verify`). EDG prints no warning on any of them (only #187 on `(x = 2) && a;`).

| shape (discarded) | clang C/C++ | gcc C | g++ | cerastes before | after |
|---|---|---|---|---|---|
| `a && g(x);`, `a \|\| g(x);` | - | unused-value | - | `value_not_used` | same |
| `c ? g(x) : h(y);`, `c ? g(x) : 0;`, `(void)(a && g(x));` | - | - | - | accepted by the encoder (calls refused later until M4.3b) | accepted |
| `c ? v(x) : w(y);`, `c ? (void)g(x) : (void)h(y);` (void arms) | - | - | - | `unsupported_type` (`void`) | same (VIR has no void value) |
| `a && (g(x), 1);` | unused-value | - | - | `value_not_used` | same |
| `a && (y = g(x));`, `a && g(x) && h(x);`, `g(x) && g(a);` | - | unused-value | - | `value_not_used` | same |
| `g(x) && a;`, `g(x) \|\| a;`, `x++ && a;`, `(x = 2) && a;`, `a && x++ && a;` | - | unused-value | - | **accepted** | `value_not_used` |
| `x++ && (y = 1);`, `x++ && a && (x = 1);` | - | - | - | accepted | accepted |

GCC's C rule (`warn_if_unused_value`): a discarded `&&`/`||` warns unless its right operand, after C's conversion to a
truth value, still has a side effect at its top, which only an assignment of a constant keeps; the left operand does not
matter. M4.3a's detector applied the right-operand test only when the right operand itself had an effect, so it accepted
the left-operand shapes: corrected in `discarded_value_with_effect` (edg-backend, M4.3b), which now reports at the
operator whose right operand fails.

## M4.4a: no wrapping, and the constants EDG folds

Probe P38 for the M4.4a decision (no wrapping, handoff §11 question 13; record `0022-no-wrapping.md`). Evidence is under `build/m4.4a/probes/p38/` (gitignored: 274 probe files, one `.ver`, `.tree`, `.err` per file and mode, plus the raw `il_display` dumps this section quotes). `il_def.h` lines are in `third_party/edg/src/il_def.h` at the pin `f0ed5525af`; every EDG source line below is in `third_party/edg/src/`; `encode.cpp` lines are those of HEAD `b395a2b` (`src/backend/encode.cpp`). The question: decision 0010 item 12 says a folded constant is checked by encoding what was written (`a_constant.expr`, recursively), and was probed only for `0u - 1` in a function body (P3). Once an unsigned operation that does not fit is an error, any place where EDG folds a wrapping constant **without** `expr` would let the wrap through as "verified". So: in which places does a folded wrapping constant keep `expr`, and where does the encoder trust EDG's value?

**Commands** (each probe file was run in the mode of its extension: `.c` as C23, `.cpp` as C++20; the generators wrote `.c` and `.cpp` copies of every shape that is valid in both).

```text
scripts/cerastes-fe.sh --c23|--c++20 --display_error_number --il_display <file>     IL evidence
scripts/cerastes-fe.sh --c23|--c++20 --cerastes_walk <file>                          typed listing of annotations
build/cerastes verify <file> --std=c23|c++20                                         today's encoder (no --keep-vpr)
```

Scratch tools (gitignored, `build/m4.4a/`): `gen38.py`, `gen38b.py`, `gen38c.py`, `gen38d.py`, `gen38e.py`, `gen38f.py`, `gen38g.py` write the probe files (`gen38b.py` combines one function per expression into one file per place and writes `probes/p38/index.json`: file to function to line range); `run38.py [-j N] <dir|file>...` runs the three commands above per file; `p38tree.py <il> fn|vars|enums|attrs|aggs|targs` condenses an `il_display` dump (modes: function bodies and `switch-case-entry`s, file-scope variables, enumerators, routine attribute arguments, aggregate initializer elements, template argument lists) and prints **every constant as `K<type value expr=NONE|expr=<tree> [orig_type=T] [local_expr_ref] [enumerator]>`**, recursively, so a missing `expr` is visible; `sum38.py` and `sum38b.py` summarise `.tree`/`.ver` per file and per function (verdict by the error lines inside the function's line range); `attrsum38.py <place>` summarises the contract trees; `table38.py`, `targs38.py`. The condenser was checked on `t0.c` against the raw dump (below). The addresses in a raw dump change with every run; the quoted blocks are identified by content.

### P38: **PASS for every place the encoder can reach; two classes of constants lose `expr`, not reachable today (noted for M5 (templates) and M4.5e (arrays) in handoff §8)**

**1. The shapes, by constant** (`fn/ret.c`, `fn/ret.cpp`, `extra/ret.c`; the same top constant in every body place, see 2). Column "IL": the folded constant's value and the root of its `expr` (`orig=` the `orig_type`). EDG's own diagnostics, `fn/ret.c` (same in C++): #68 on e10, #69 on e13, #63 on e18 and e21, **none on any other row** (`unsigned` `+ - *`, `<<` that loses bits, `-1u`, explicit casts and sizeof/limits macros are silent). "Verify today" is `build/cerastes verify`: every row in every place verifies except the two shift counts (`1u << 32`, `1 << 40`), which `shift` rejects today and which serve as the **witness** that the written expression, not EDG's value, is what the encoder checks.

| # | expression (as returned/assigned as `T`) | T | IL (C23) | IL (C++20) | verify today |
|---|---|---|---|---|---|
| e01 | `-1u` | unsigned | `4294967295U` `eok_negate` | same | verified |
| e02 | `0u - 1u` | unsigned | `4294967295U` `eok_subtract` | same | verified |
| e03 | `0u - 1` | unsigned | `4294967295U` `eok_subtract` (the `1` is a generated cast) | same | verified |
| e04 | `4294967295u + 1u` | unsigned | `0U` `eok_add` | same | verified |
| e05 | `65536u * 65536u` | unsigned | `0U` `eok_multiply` | same | verified |
| e06 | `1u << 31` (fits) | unsigned | `2147483648U` `eok_shiftl` | same | verified |
| e07 | `3u << 31` (**loses a bit**: `3 * 2^31 = 0x180000000`) | unsigned | `2147483648U` `eok_shiftl` | same | verified |
| e08 | `6u << 31` (loses bits) | unsigned | `0U` `eok_shiftl` | same | verified |
| e09 | `(unsigned)-1` | unsigned | `4294967295U` `eok_cast` `orig=int` | same | verified |
| e10 | `-1` (implicit, `unsigned u = -1`) | unsigned | `4294967295U` `eok_cast` (generated) `orig=int` | same | verified (#68) |
| e11 | `-(1u)` | unsigned | `4294967295U` `eok_negate` | same | verified |
| e12 | `0ull - 1` | unsigned long long | `18446744073709551615ULL` `eok_subtract` | same | verified |
| e13 | `300` (implicit, `unsigned char c = 300`) | unsigned char | `44U` `eok_cast` (generated) `orig=int` | same | verified (#69) |
| e14 | `(unsigned char)-1` | unsigned char | `255U` `eok_cast` `orig=int` | same | verified |
| e15 | `(0u - 1u) > 5u` | int | `1` `eok_gt` | `1` `eok_cast` `orig=bool` over the `eok_gt` | verified |
| e16 | `(0u - 1u) / 2u` | unsigned | `2147483647U` `eok_divide` over the `eok_subtract` constant | same | verified |
| e17 | `4294967294u + 1u` (fits, control) | unsigned | `4294967295U` `eok_add` | same | verified |
| e18 | `1u << 32` | unsigned | `0U` `eok_shiftl` | same | `shift` (#63) |
| e19 | `(unsigned char)300` | unsigned char | `44U` `eok_cast` `orig=int` | same | verified |
| e20 | `-(0u)` (fits, control) | unsigned | `0U` `eok_negate` | same | verified |
| e21 | `1 << 40` (witness) | int | `0` `eok_shiftl` | same | `shift` (#63) |
| x01 | `sizeof(int) - 5` | unsigned long | `18446744073709551615UL` `eok_subtract` | same | verified |
| x02 | `sizeof(char) - 2` | unsigned long | `18446744073709551615UL` `eok_subtract` | same | verified |
| x03 | `UINT_MAX + 1u` | unsigned | `0U` `eok_add` | same | verified |
| x04 | `ULONG_MAX + 1ul` | unsigned long | `0UL` `eok_add` | same | verified |
| x05 | `SIZE_MAX + 1` | unsigned long | `0UL` `eok_add` | same | verified |
| x06 | `UCHAR_MAX + 1` | int | `256` `eok_add` | same | verified |
| x07 | `(unsigned char)(UCHAR_MAX + 1)` | unsigned char | `0U` `eok_cast` `orig=int` | same | verified |
| x08, x10, x11, x12 | `sizeof(int) * 3 - 12`, `UINT_MAX - 0u`, `INT_MAX + 1u`, `UINT32_MAX + 1` (fit, controls) | | `0UL` `eok_subtract`, `4294967295U` `eok_subtract`, `2147483648U` `eok_add`, `0UL` `eok_cast` `orig=unsigned int` | same | verified |
| x09 | `(unsigned)sizeof(char) - 2u` | unsigned | `4294967295U` `eok_subtract` | same | verified |

**Raw dump** (`probes/p38/t0.c`, line 1 `unsigned r(void) { return 0u - 1u; }`, `scripts/cerastes-fe.sh --c23 --display_error_number --il_display t0.c`, the constant of the `return`, with the operands of its `expr`):

```text
func-scope constant@...5b0
type:                    file-scope type@...: unsigned int
expr:                    func-scope expr-node@...4d0
kind:                    ck_integer
integer_value:           4294967295U
func-scope expr-node@...4d0   kind: enk_operation   operation.kind: eok_subtract   (operands: constant 0U, constant 1U, both without expr: literals)
```

`il_def.h:4777` `orig_type`, `:4781-4790` `expr` ("If the constant is not just a literal this points to an expression node representing that constant. Otherwise, NULL ... (Note that for some implicitly-converted constants, this field is NULL and the conversion's original type is recorded in orig_type instead.)"). **No constant in any of the 274 files has `local_expr_ref`** (`il_display.c:1355`; `p38tree.py` prints the flag when set, and `grep -rl local_expr_ref probes/p38` finds no file).

**2. The places** (every cell is evidence from `probes/p38`; "kept" means the `expr` field is present and the tree under it is the written expression, checked in each of the listed files by `p38tree.py`; the summary lines are `sum38b.py <place>` and `sum38.py <dir>`).

| place | files (line of e02/e13) | IL: `expr` kept for the folded wrapping constant? | `orig_type` without `expr`? | encoder path today | verify today (C23 / C++20) |
|---|---|---|---|---|---|
| **body: initializer, assignment, return, argument, condition** | `fn/init.c:4`, `asg`, `ret.c:4`, `arg`, `cnd` (and `.cpp`), e01-e21 and `extra/ret.c` x01-x12 | **kept** on e01-e21 in all 5 places and both modes (210 function results in `fn/{init,asg,ret,arg,cnd}.{c,cpp}`) and on x01-x12 in `extra/ret.*` and `extra/cnd.*` (48 results); no top-level constant of any of these rows lacks `expr` (`sum38b.py fn extra`; its `SUSPECT` column lists only macro leaves under a kept `expr`) | none | `constant` (`encode.cpp:681`) to `constant_value` (`:712`), the `original != nullptr` branch (`:757-781`): the expression is encoded as written | every row verified, but e18 and e21: `shift` (`fn/ret.c:20`, `:23`; the same in the other four places and in `.cpp`) |
| **contract `requires`/`ensures`** (il_display prints routine attributes: `p38tree.py attrs`) | `fn/req.c:6`, `fn/ens.c`; `whole/req.c`, `whole/ens.c` | **no folded constant lost its `expr`**: in a contract EDG either leaves the operation unfolded (`eok_eq(param_ref, eok_subtract(0U, 1U))`: no constant at all) or folds with `expr` kept (C only, and only some second operands: `unsigned int=4294967295U expr=eok_subtract(...)`, `attrsum38.py req`). Fully constant annotations are not folded at all (`whole/req.c`: `attr requires args: eok_eq[int](eok_shiftl[int](const 1, const 40), const 0)`) | none | the argument tree is encoded by `expr`, a folded constant by the same `constant_value` path | e18, e21 and `whole/`'s `1 << 40 == 0`: `shift` in `requires` (`whole/req.c:4`), `ensures` (`whole/ens.c:4`); all other rows verified. **These `shift` errors prove nothing about folded constants: EDG does not fold the shift in an annotation (the tree above is `eok_shiftl` over two constants), so the encoder reports the operation itself** (audit round 1). The folded constants of annotations are covered by the hook, below. C++: the second operand never folds (only the negations show a constant, each with `expr=eok_negate`: `-1u`, `-(1u)`, `-1` under a cast, `-(0u)`) |
| **contract `check`, `invariant`/`decreases`** | `fn/chk.c:7`, `fn/inv.c`; `whole/chk.c`, `whole/inv.c` | **il_display does not print statement attributes** (M1: `attributes:` names the entry and the entry is not dumped), so no `expr` listing exists. Indirect evidence: the walker (`--cerastes_walk`, `fn/chk.c.c23.walk`) shows the same shapes (`op eok_subtract` over `const 0`, `const 1` for the first operand, `const 4294967295` for a folded second one) but prints only values; the `shift` of `CERASTES_CHECK(1 << 40 == 0)` (`whole/chk.c:4`, `whole/inv.c:5`) is, as for `requires`, the unfolded operation and proves nothing about folded constants; the hook below covers `check` and `invariant` | not observable in `il_display` | same path (the annotation expression is scanned like any expression, `ek_normal`) | e18/e21 `shift` (unfolded operations); the rest verified |
| **file-scope named constants** `const T K = E;` (decision 0017) | `named/nc.c:5`, `nc.cpp`, `one_e18`, `one_e21` | **kept**: the initializer constant of `K` has `expr` for every row (`p38tree.py vars`: `K_e02: ... constant=K<unsigned int 4294967295U expr=eok_subtract[...]>`); a use `return K;` is a constant with `expr=var K` | none (e13: `K<unsigned char 44U expr=eok_cast {gen}(K<int 300 expr=NONE>) orig_type=int>`) | prologue: `encode_named_variable` (`:940`) to `constant_initializer` (`:1650`) to `constant_value` path A; a use is the literal of EDG's value (`variable()`, `encode.cpp:835`) after that check | verified; e18 / e21 alone: `shift` at the declaration. **A failing initializer hides the unit's other errors** (0017; the first combined file hid e21 behind e18's error, so they are probed alone) |
| **`constexpr` named constants** (C++) | `named/ncx.cpp`, `onex_e18.cpp`, `onex_e21.cpp` | **kept**, same as `const` | none | same | verified; e18/e21: EDG's own error #28 (`expression must have a constant value`), the whole unit stops in the front end |
| **named constants made of named constants** | `named/nest_use.cpp`, `nest_wrapK.cpp`, `nest_use_constexpr.cpp`, `nest_wrapK_constexpr.cpp`, `nest_*_code.*`, `nest_B.*`, `nest_Bcode.*`, `nest_uc.*`, `nest_ucK.*` | **kept** (`const unsigned K2 = K + 1u;`: `K<unsigned int 0U expr=eok_add[...](var K, 1U)>`; a use in code `K + 1u`: `0U expr=eok_add`; `const int B = (0u-1u) > 5u;`: `1 expr=eok_gt`/`eok_cast orig=bool`) | none | same | verified; C: `nest_use.c`/`nest_wrapK.c` are EDG error #1901 (a `const` object in a constant expression, decision 0017) |
| **enumerators with a written initializer** (decision 0018) | `enum/n01`-`n08`, `n11`, `n12`, `n14`, `n19`-`n21`, `n23`-`n26`, `n31`, `n32` (`.c`, `.cpp`) | **kept**: the enumerator constant (`is_named_constant_definition`) has `expr` for every initializer that is an expression, e.g. `n02` `A = (0u - 1u) / 2u`: `K<int 2147483647 expr=eok_divide[unsigned int](K<unsigned int 4294967295U expr=eok_subtract ...>, 2U) enumerator>`; `n03` `(unsigned char)300`: `K<int 44 expr=eok_cast[unsigned char](K<int 300>) orig_type=int enumerator>` (C++: enumerator type is the enumeration) | none | `encode_named_enumerator` (`:1041`, `expr_node_from_constant` at `:1047`): the initializer is encoded as written in the prologue | verified for the ones whose value fits `int` (n01-n08, n12, n14, n19, n20, n23, n24, n26, n31, n32); n11 `(int)(0u - 1u)`: `conversion` today; n13 `1 << 40`: `shift` (C) / EDG #28 (C++); n17, n21, n25: `unsupported_declaration` (value outside `int`, 0018 item 2) |
| **enumerators without `expr`** (the second trust site, `encode.cpp:1046`) | `enum/n09`, `n10`, `n27`-`n30` (literal initializer, also converted to a fixed base), `n33`-`n40` (implicit values) | **no `expr`, no `orig_type`**, value is the literal or previous + 1: e.g. `n27` `enum : unsigned { A = 5 }`: `enum <unnamed> 5U expr=NONE enumerator`. The value cannot wrap: for a fixed base EDG itself refuses (**#1749**, `n15`, `n16`, `n18`, `n22`, `n33`, `n35`, `n36`, `n38`-`n40`, the wrapping implicit value and the literal outside the type); for an unfixed one the type widens (`n34`, `n37`: `4294967296L`, `2147483648U`) and the value is outside `int`, which 0018 item 2 rejects | none | `if (c->expr == nullptr && !c->local_expr_ref) return;` (`:1046`): nothing is checked, EDG's value is used | verified for fitting literals (n09, n10, n27-n32); rejected as above |
| **`case` values** (C and C++) | `case/k01`-`k18` | **kept** for every case constant that is not a plain literal: `k01` (C) `case -1:` over `unsigned`: `K<unsigned int 4294967295U expr=eok_cast {gen}(K<int -1 expr=eok_negate(...)>) orig_type=int>`; `k02` `0u - 1u`: `expr=eok_subtract`; `k04`, `k05`, `k06`, `k09`, `k10`, `k11`, `k14`, `k16`-`k18` likewise; plain literals (`k03` `4294967295u`, `k07` `300`) have `expr=NONE` and no `orig_type` | none | `switch_statement` (`:2517`) to `constant_value` (`:2645`); a bare `enum` switch in C through `enum_case_value` (`:2473`) | verified; `k01.cpp`: EDG #2362 (narrowing); `k13` `1 << 40`: `shift`; `k15` (`unsigned char x`, `case -1`) verified (`int` promotion, no wrap); `k16.c`: #1901 |
| **`case` over an enumeration with a fixed base** (the enum-typed case constant of `enum_case_value`) | `ecase/c01`-`c15` | every case constant that is not a literal has `expr` (e.g. `c05.c` `enum E=4294967295U expr=eok_cast orig_type=int`); EDG leaves `case 300:` over `enum E : unsigned char` as a bare `int` (`ecase/c01`); rule 1 (`case_not_in_enumeration`) compares EDG's value, and `c12` (`X = 44`, `case 300`) and `c14` still reject (the case value 300 / -1 is not 44 / 255); an explicit cast into the enumeration (`c13`, `c15`) is the `conversion` error today (the value is checked, never wrapped, 0018 item 6) | none | `enum_case_value`: the generated cast is peeled and the written constant is encoded; the `!has_expr` arm (`:2476`: an enumerator or an enum-typed `ck_integer` without `expr`) delegates to `constant_value` (`:2494`), which rejects `orig_type` without `expr` (`:783-787`), and since audit round 1 rejects it first itself; P38 reaches the arm only with enumerator copies (`c11` `case X:`, `enum E=1U expr=NONE enumerator`) | `case_not_in_enumeration` (exit 2) for `c01`-`c10`, `c12`, `c14`; `conversion` for `c13`, `c15`; `c11` verified; C++ `c05`, `c06`: EDG #2362 |
| **default arguments** (C++) | `defarg/d01.cpp`-`d13.cpp` | **kept**: the default-argument constant (`generated_default_arg`) has `expr` (`defarg/d01.cpp`, `scripts/cerastes-fe.sh --c++20 --il_display`: `file-scope constant@...: expr: file-scope expr-node@... kind: enk_operation eok_subtract`, integer_value `4294967295U`); `d02` `unsigned a = -1`: `eok_cast orig_type=int` | none | the call's argument node is a constant with `generated_default_arg`; `constant_value` path A, reported at the call (`encode.cpp:1298-1300`) | verified; `d09` `int a = 1 << 40`: `shift` |
| **locals, automatic and static** | `loc/l01`-`l10`, `g01`-`g08` | **kept**: `const unsigned char c = 300;` (local) `K<unsigned char 44U expr=eok_cast {gen}(...) orig_type=int>`; file-scope `const unsigned long long K = 5;` `expr=eok_cast orig_type=int` | none | as above | automatic verified; `static` locals: `unsupported_declaration` (0012); `g08` `const short K = 70000;`: `conversion` |
| **`static_assert`/`_Static_assert`, `#if`, array bounds** (EDG's, 0010 item 13) | `misc/sa.*`, `pp.*`, `arr.*` | not encoded | | | `_Static_assert((0u - 1u) > 5u, ...)` and `_Static_assert(65536u * 65536u == 0u, ...)`: **no diagnostic, accepted** (`misc/sa.c`, `sa.cpp`); `#if (0u - 1u) > 5u` and `#if -1 > 0u`: both true, wrapping (the preprocessor's own modular `uintmax_t`), only #68 on the sign change of `-1 > 0u` (`misc/pp.c:8`); array bound `int a[(0u - 1u) > 5u ? 1 : 2]`: EDG gives 1, the array is `unsupported_declaration` (`misc/arr.c`) |
| **non-type template arguments** (C++) | `tmpl/t01`-`t09`, `targs38.py` | **LOST** (class 1 below) | **both**: with a conversion `orig_type` and no `expr`; without one neither | not reached: the template is rejected first (`unsupported_declaration`, "a template is not supported", `tmpl/t01.cpp:3`; `t02` "a class, struct or union ..."; `t03` "a global declared in a system file (`value`...)") | `t01`, `t04`-`t07`, `t09`: `unsupported_declaration`; `t08` `t<-1>`: EDG #304 / #3323 |
| **aggregate initializers** (arrays, structs) | `misc/agg_short.*`, `agg_neg.*`, `agg_long.*` | **kept** for lists of at most 1000 elements (`agg_short`: `const unsigned char A[3] = {300,1,1}` elements `K<unsigned char 44U expr=eok_cast {gen}(K<int 300>) orig_type=int>`); **LOST** beyond that (class 2 below) | **yes** in `agg_long` | not reached: arrays are `unsupported_declaration` (0012; M4.5e) | rejected at the declaration |

**3. Where the encoder takes EDG's value, and why each place is covered** (the list of 0022 item 3; `encode.cpp` lines of HEAD `b395a2b`).

| place | what the encoder takes | why a wrapping constant cannot get through |
|---|---|---|
| a literal without `expr` (`constant_value`, `:789`) | EDG's value of a `ck_integer` with `expr == NULL`, `orig_type == NULL`, not `local_expr_ref` | it is the number written: in P38 the constants that reach it are only plain literals (`0`, `1`, `5u`, `300`, `4294967295u`, `31`, `2u`, the `1`/`0` of generated `return` values and the leaves inside the macro bodies of `<limits.h>`); the census `sum38.py`/`sum38b.py` makes of constants with `expr=NONE` and a value that is not a number of the source text (`SUSPECT`, all 274 files) lists only macro leaves (under a kept `expr`), the implicit enumerator values of `enum/n34`/`n37` (the next row), and aggregate and template-parameter constants rejected earlier. **No folded wrapping constant of section 2 reaches this arm.** The only shapes that would (class 1, template arguments) have no path today |
| a constant with `orig_type` and no `expr` (`:783-787`) | nothing: rejected ("a folded constant whose original expression is not recorded") | reached in P38 only by shapes rejected earlier (`tmpl/t05`, `t07` through their instances, `agg_long`) |
| a use of a named constant or an enumerator (`variable()` `:835`, `is_enumerator` in `constant_value`) | EDG's value of the initializer | the initializer is encoded as written and checked in every method's closed prologue (0017, 0018), where a failed obligation ends the path (S14, S15) before any use |
| an enumerator without an initializer (`encode_named_enumerator`, `:1046`) | EDG's value: a literal initializer, or the previous value + 1 | no wrap is possible: for a fixed base EDG itself refuses (#1749: `n15`, `n16`, `n18`, `n22`, `n33`, `n35`, `n36`, `n38`-`n40`); for an unfixed one the type widens (`n34`, `n37`: `4294967296L`, `2147483648U`) and the value is outside `int`, which 0018 item 2 rejects; the implicit values are checked to be previous + 1 (0018) |
| `sizeof` / `alignof` | their value, no run-time operation (0010 item 12) | a layout fact, not an arithmetic result (`x01`, `x02` keep the surrounding subtraction as `expr`) |
| `Case::folded`, the value of a `case` label | EDG's value for the dispatch | guarded in the prologue: the value as written, with its obligations at the label, must equal EDG's (0017); `k01`-`k18`, `ecase/c01`-`c15` |
| an enumerator copy as a case value (`enum_case_value`, `:2476`) | the enumerator's value, converted to the promoted type | an enumerator or an enum-typed `ck_integer` without `expr`; the arm delegates to `constant_value`, which rejects `orig_type` without `expr`, and checks `orig_type` first itself since audit round 1; P38 reaches it only with `case X:` copies (`ecase/c11`), whose value is the enumerator's and was checked against its enumeration (0018) |

**The annotation hook** (audit round 1; closes the contracts rows of section 2). `il_display` does not print the arguments of `check`/`invariant`, and the shift witness proves nothing in annotations (EDG leaves the shift unfolded there). The encoder's own hook does: `--cerastes_verify_selftest=constant_without_expr` makes `constant_value` ignore every `expr` (`:758-789`), so it rejects each folded constant that has one (`error #cerastes-unsupported_expression: a folded constant whose original expression is not recorded ...`). `build/m4.4a/hook38.py` runs `build/cerastes verify probes/p38/fn/{chk,inv,req,ens}.{c,cpp} --std=c23|c++20 --cerastes_verify_selftest=constant_without_expr` (outputs `probes/p38/hook/*.out`, `hook/summary.txt`; the auditor's runs are in `/tmp/audit-m44a/raw/`): every run exits 2 and rejects, by function,
- **C23: 15 of 21 functions** in each of the four places: e01, e02, e04, e05, e06, e07, e08, e09, e10, e11, e14, e15, e16, e17, e20;
- **C++20: 6 of 21** in each: e01, e09, e10, e11, e14, e20 (the negations, the only constants EDG folds in a C++ annotation).

For `requires` and `ensures` these are exactly the functions whose `il_display` shows a folded constant with an `expr` (`attrsum38.py req`: the same 15 and the same 6), and `check` and `invariant` give the identical sets, so every folded constant of an annotation has an `expr` in both modes. (What the hook cannot show is a folded constant with neither `expr` nor `orig_type`: it would be taken as a literal. None is visible in `req`/`ens`, and `chk`/`inv` scan the same expressions through the same scanner.)

### The two classes that lose `expr`

**Class 1: non-type template arguments (not reachable today; noted for M5 in handoff §8).** `il_display` of `tmpl/t01.cpp` (`unsigned f(void) { return t<0u - 1u>(); }`, `template <unsigned N> unsigned t(void) { return N; }`), the instance routine `t`:

```text
file-scope routine@...: t        assoc_template: file-scope template@...   is_template_function: TRUE
template_arg_list:
  constant:              file-scope constant@...0980: 4294967295U
  explicitly_specified:  TRUE
file-scope constant@...0980:  type: unsigned int   null_pointer_constant_ruled_out: TRUE   kind: ck_integer   integer_value: 4294967295U
                              (no `expr:` line, no `orig_type:` line)
```

The instance body's `return N;` is an `enk_constant` naming a *different* constant, `constant@...0f50: 4294967295U` (second `stmk_return` of region `t`, `tmpl/t01.cpp.c++20.tree`); `il_display` does not dump that constant's block, so its `expr` **cannot be read: UNCLEAR** (the dumped one, the template argument, has none; the class-template member initializer of `t02` is dumped and has none, below). `targs38.py` over `tmpl/t0*.cpp`: `t<0u - 1u>`: `K<unsigned int 4294967295U expr=NONE>`; `t<65536u * 65536u>`: `0U expr=NONE`; `t<4294967295u + 1u>`: `0U expr=NONE`; `t<5u>`: `5U expr=NONE`; with a conversion: `t<(unsigned char)300>`: `K<unsigned char 44U expr=NONE orig_type=int>`; `t<(0u - 1u) / 2u>` with `int N`: `K<int 2147483647 expr=NONE orig_type=unsigned int>`; the class-template member `S<0u - 1u>::v` (`tmpl/t02.cpp`): `v: ... constant=K<unsigned int 4294967295U expr=NONE>`. **Cause:** `BACKING_EXPR_FOR_NONTYPE_TEMPL_ARG` is FALSE in the cerastes configuration (`host_envir.h:1533-1545`: TRUE only `#if BACK_END_IS_CP_GEN_BE`, which `edg_config` sets to 0, `cmake/EdgFrontEnd.cmake`), so `curr_expr_kind_is_one_in_which_const_exprs_are_recorded()` (the arm for `!BACKING_EXPR_FOR_NONTYPE_TEMPL_ARG`, `exprutil.h:1375-1381`: `!ek_pp && (depth_template_declaration_scope != NO_SCOPE_DEPTH || !curr_expr_kind_is(ek_template_arg))`) is FALSE for `ek_template_arg` outside template declarations, and `expr_interpret_expression_operand` clears it (`exprutil.c:6431-6433`); the comment at `host_envir.h:1520-1528` says why (one instance is referred to by many expressions that fold to the same value; only one could be kept). **The shape cannot be recognised from the constant alone:** with no conversion it is `ck_integer`, `expr == NULL`, `orig_type == NULL`, `local_expr_ref` FALSE, exactly a literal; the only difference is where it sits (the `template_arg_list` of an instance routine or class, or the body/members of such an instance). With a conversion it carries `orig_type` and is rejected by the existing `orig_type` rule (`:783-787`). **No path today:** cerastes rejects templates (`unsupported_declaration` at the prototype, M4) and system class-template members (`t03`), so `constant_value`'s literal arm never sees one. M5 (generic templates, instances verified per instantiation) will, and then `BACKING_EXPR_FOR_NONTYPE_TEMPL_ARG=1` is not enough (one expression per instance). The instance-specific value would need the written argument from the call's `expl_template_arg_list_used` / source positions, which P38 did not probe.

**Class 2: simple implicit conversions of constants in long braced initializers (not reachable today; noted for M4.5e in handoff §8).** `REDUCE_BACKING_EXPRESSION_USE` is TRUE (`host_envir.h:1494-1496`); `decl_inits.c:4722-4725` sets `reduce_backing_expression_use` when `dps->init_state.pending_elements` (a braced list longer than `MIN_BRACED_INIT_PARSE_ELEMENT_COUNT` = 1000 elements, `expr.c:47559`, `:47693`, `:48011`), and `exprutil.c:10531-10546` then records **no backing expression** for "a nontrivial implicit cast on top of a simple constant ... a top-level initializer expression for an entity with static storage duration", only `orig_type` (`exprutil.c:10518-10526`). `misc/agg_long.c` (`const unsigned char A[1101] = { 300, 1, ... }`, 1101 elements) `p38tree.py aggs`: `aggregate const unsigned char [1101]: 1101 elements; 1101 with orig_type and no expr`, first element `K<unsigned char 44U expr=NONE orig_type=int>` (raw: `first_constant: constant@...: (unsigned char)44U`, `orig_type: int`, `integer_value: 44U`, no `expr:`), while `agg_short.c` (3 elements) gives `K<unsigned char 44U expr=eok_cast[unsigned char] {gen}(const K<int 300 expr=NONE>) orig_type=int>`. **Recognisable:** `orig_type != NULL && expr == NULL`, which `constant_value` already rejects (`:783-787`), so an implementation of M4.5e that routes every element through `constant_value` is safe; one that reads `ck_aggregate` elements' values directly (`first_constant`/`next`) is not.

**Not classes of loss:** `ek_pp` (`#if`, never in the IL, EDG's); `_Static_assert` and array bounds (EDG's, accepted silently as shown above); enumerators without `expr` (no wrap possible, see the table); `local_expr_ref` (never seen).

**Verdict: PASS** for the places the encoder reaches today (body, contracts, named constants, enumerator initializers, case values, default arguments, locals): a folded wrapping constant keeps `expr` there in both modes, and the encoder encodes it as written. In function bodies, named constants, case values and default arguments the `1 << 40` witness gives `shift` (EDG folds it there and keeps `expr`); in annotations the evidence is the hook (above), not the witness. **Not reachable today:** class 1 (template arguments; noted for M5 in handoff §8) and class 2 (long aggregate lists; noted for M4.5e in handoff §8); the first is not recognisable from the constant. Surprises: `3u << 31` does not fit in 32 bits (`1u << 31` is the fitting control); EDG warns on none of the wrapping unsigned arithmetic (only #68/#69 on conversions and #63 on shift counts), so the encoder is the only check once wrapping is an error; a failing named-constant initializer hides the others (0017).

### P39: which `==`/`!=` with an enumeration operand the compilers warn on (0022 item 5): **PASS**

`build/m4.4a/probes/p39/sc.c` (22 functions, `s1`-`s22`, line 5 + n; `run.sh` strips the `#include` to make `sc.cpp`, keeping the line numbers); `bash build/m4.4a/probes/p39/run.sh` (output `probes/p39/run.out`): Apple clang 21.0.0 (`clang-2100.3.34.2`) and GCC 16.2.0 (`gcc-16`), `-Wall -Wextra -fsyntax-only`, `-std=c23` in C and `-std=c++20` in C++. The declarations: `enum color { RED, GREEN, BLUE }` (unsigned compatible type in C), `enum neg { NA = -1, NB }`, `enum fu : unsigned { FA, FB }`, `static const int M = -1`. `W` is `-Wsign-compare`; `T` is clang C++'s `-Wtautological-constant-out-of-range-compare` ("comparison of constant -1 with expression of type 'enum color' is always false"); `-` no warning. "Verdict" is what 0022 item 5 decides, the same in both modes.

| # | shape | clang C | clang C++ | gcc C | gcc C++ | verdict (item 5) |
|---|---|---|---|---|---|---|
| s1 | `c == x` (`int x`) | W | - | W | - | rejected |
| s2 | `c != s` (`short`) | W | - | W | - | rejected |
| s3 | `c == v` (`signed char`) | W | - | W | - | rejected |
| s4 | `c == v` (`char`, signed on this target) | W | - | W | - | rejected |
| s5 | `c == -1` | W | T | W | - | rejected (negative constant) |
| s6 | `c == 1` | - | - | - | - | accepted (a constant >= 0, item 4) |
| s7 | `c == M` (`const int M = -1`) | W | T | W | - | rejected (negative named constant) |
| s8 | `x == (enum color)2` | - | - | - | - | **rejected, stricter than the compilers** (the other operand is a signed `int` that may be negative) |
| s9 | `c == b` (`bool`) | - | - | - | - | accepted |
| s10 | `c == l` (`long`) | - | - | - | - | accepted (C compares in `long`) |
| s11 | `c == (int)u` | W | - | W | - | rejected |
| s12 | `c == x + 1` | W | - | W | - | rejected |
| s13 | `f == x` (`enum fu : unsigned`) | W | W | W | W | rejected (a fixed `unsigned` base is compared unsigned in both modes) |
| s14 | `n == x` (`enum neg`) | - | - | - | - | accepted (an enumeration with a negative value is signed) |
| s15 | `x != c` | W | - | W | - | rejected (either order) |
| s16 | `c == uc` (`unsigned char`) | - | - | - | - | accepted |
| s17 | `c == (x > 0 ? 1 : 2)` | - | - | - | - | **rejected, stricter than the compilers** (an `int` that is not a constant) |
| s18 | `c == d` (two `enum color`) | - | - | - | - | accepted |
| s19 | `(int)c == x` | - | - | - | - | accepted (the explicit cast gives an `int`) |
| s20 | `GREEN == x` | - | - | - | - | accepted (an enumerator) |
| s21 | `c == (enum color)x` | - | - | - | - | accepted (two enumeration values; the cast itself is a checked conversion) |
| s22 | `c == -x` | W | - | W | - | rejected |

Counts: clang C 11 warnings (s1-s5, s7, s11, s12, s13, s15, s22), clang C++ 3 (s5 and s7 as `T`, s13 as `W`), gcc C the same 11 as clang C, gcc C++ 1 (s13). So both compilers, in C only, warn on every shape where a signed operand that may be negative meets an enumeration without negative values, and in C++ only on the fixed `unsigned` base, which is item 5's rule. The two shapes the rule rejects although no compiler warns are **s8** and **s17**: both compare an enumeration value with a signed `int` that is not a constant >= 0, so in C a negative `int` would be a `conversion` error in C only; the rule stays simple and uniform (0022 item 5, "Stricter than the compilers"). Item 5's verdict column was derived from the rule's four conditions applied to each shape, not run through the encoder (the encoder change is edg-backend's).

**P39 addendum (audit round 2): two more probe files, the auditor's** `/tmp/audit-m44a/h1.{c,cpp}` and `g.{c,cpp}` (copied to `build/m4.4a/probes/p39/audit/`; `bash build/m4.4a/probes/p39/run2.sh`, output `probes/p39/run2.out`; the same compilers and flags). The verdicts are 0022 item 5's (the rule applied to each shape, not run through the encoder).

*Two different enumerations* (`h1.c`; the line numbers are the functions' `L4`-`L7`; declarations as in `sc.c`):

| # | shape | clang C | clang C++ | gcc C | gcc C++ | verdict |
|---|---|---|---|---|---|---|
| h1 | `c == n` (`enum color`, `enum neg`) | `-Wenum-compare` and W | `-Wdeprecated-enum-compare` | `-Wenum-compare` and W | `-Wenum-compare` | rejected in both modes |
| h2 | `c == NA` (an enumerator of another enumeration) | `-Wenum-compare` and W | `-Wdeprecated-enum-compare` and T | `-Wenum-compare` and W | `-Wenum-compare` | rejected in both modes |
| h3 | `f == n` (`enum fu : unsigned`, `enum neg`) | `-Wenum-compare` and W | `-Wdeprecated-enum-compare` and W | `-Wenum-compare` and W | `-Wenum-compare` and W | rejected in both modes |
| h4 | `f == NA` | `-Wenum-compare` and W | `-Wdeprecated-enum-compare` and W | `-Wenum-compare` and W | `-Wenum-compare` and W | rejected in both modes |

Every compiler warns on every shape in both modes (`-Wenum-compare` in C and in g++, `-Wdeprecated-enum-compare` in clang++), so item 5 rejects a comparison of values or enumerators of two different enumerations in both modes (`c == n` would otherwise be a `conversion` error in C and verify in C++). Two values of one enumeration stay accepted (`s18`, `s21`).

*Truth-valued and constant operands* (`g.c`; `g1` is on line 3, `g10` on line 12 in both files):

| # | shape | clang C | clang C++ | gcc C | gcc C++ | verdict |
|---|---|---|---|---|---|---|
| g1 | `c == (x > 0)` | - | - | - | - | accepted (a truth value is 0 or 1) |
| g2 | `c == !x` | - | - | - | - | accepted |
| g3 | `c == (x && y)` | - | - | - | - | accepted |
| g9 | `c == (x == 0)` | - | - | - | - | accepted |
| g4 | `c == 'a'` | - | T | - | - | accepted (a constant >= 0) |
| g8 | `c == (int)(0u - 1u + 1u)` (the constant 0) | - | - | - | - | accepted (a constant >= 0) |
| g5 | `c == '\xff'` (-1 on this target) | W | T | W | - | rejected (a negative constant) |
| g6 | `c == (short)-1` | W | T | W | - | rejected |
| g7 | `c == (y = x)` | W | - | W | - | rejected (a signed `int` that may be negative) |
| g10 | `c == (int)b` (`bool b`, an explicit cast) | W | - | - | - | rejected (not a truth value; clang warns, the strictest reading) |

So `g1`, `g2`, `g3` and `g9`, the four truth-valued shapes of the audit, are silent in all four configurations and accepted in both modes. `T` is clang C++'s `-Wtautological-constant-out-of-range-compare`.

## M4.4b1: bitwise operators, and what EDG leaves in the IL

Probe P40 for the M4.4b decision (bitwise operators `& | ^ ~ &= |= ^=`; record 0023) and the Q33 facts on character literals (handoff §11 question 33). Evidence is under `build/m4.4b1/p40/` (gitignored: probe sources in `probes/{ops,enum,fold,par,cnd,chr2}/`, one `.tree`, `.err`, `.walk`, and (verify) `.ver` per file and mode, `IL` dumps not kept; the scratch tools are `run40.py` (runner: `scripts/cerastes-fe.sh --c23|--c++20 --display_error_number --il_display`, `--cerastes_walk`, `build/cerastes verify`), `gen40a.py`, `gen40c.py`, `gen40q.py` (generators), `sum40.py`, `fns.py`, `uac40.py`, `chk40.py`, `col40.py`, `il40.py` (summaries and checks), `p38tree.py` (the P38 condenser, copied; a node's flags print as `{par,gen,lv,nu,olt=}`, a constant as `K<type value expr=NONE|tree orig_type= enumerator>`). `il_def.h` lines are at the pin `f0ed5525af`. The files of `.c` ran as C23, those of `.cpp` as C++20; every shape valid in both has both.

### P40: **PASS: no stop condition. Every folded bitwise constant keeps its original `expr` at every place the encoder reads; `is_parenthesized` is on the written operation node (inside a folded constant's `expr`, never on the constant)**

**1. Operand conversions** (`probes/ops/{bin,cmp,cpl}.{c,cpp}`, 32 + 98 + 14 shapes per operator and mode; `*.c23.sum`, `*.cpp20.sum` are the condensed tables; **C23 and C++20 are identical** except the C++ compound node is an lvalue (`{lv,nu}`) and C's `bool` results are `eok_bool_cast`/`eok_cast`-wrapped the same way in both):

| shape | IL (both modes) |
|---|---|
| `a OP b` (`& | ^`, same shape for the three; 96 rows) | `eok_and|or|xor[R]` where **R = the usual arithmetic conversion of the promoted operands**; each operand that is not already of type R carries a **compiler-generated `eok_cast[R]`** (so `uchar & uchar` is `and[int](cast[int](a), cast[int](b))`, `int & unsigned` is `and[unsigned](cast[unsigned](a), b)`, `long & unsigned` is `and[long](a, cast[long](b))`, `int & long long` casts a, `bool & bool` casts both to int). Checked on all 96 rows: both operands' effective type equals the operation's type. `__int128` / `unsigned __int128`: same (R is the 128-bit type, mixed signedness gives the unsigned one). |
| the use of the result | the result is **not** converted at the node: a `return`, initializer or argument adds its own generated `cast[T]` around the node, a `bool` destination `eok_bool_cast[bool]` (`f_and_bool_bool`: `bool_cast(and[int](cast[int] a, cast[int] b))`). |
| `~a` (14 types, 3 uses) | `eok_complement[R]` with R the promoted type (`int` for char, short, bool and their unsigned, else the type); a promoted operand carries `cast[int]`. |
| `a OP= b` (588 rows, `OP=` for 14 lhs types x 6-10 rhs types) | `eok_and_assign|or_assign|xor_assign[T]` where **T is the type of the left operand** (not the computed type), the **left operand is bare** (`var a {lv}`, no promotion cast), and **the right operand carries the generated cast to R = UAC(promote(lhs), promote(rhs))** when its own type differs (`uchar &= int` bare int; `uint &= int` `cast[unsigned](b)`; `int &= uint` bare; `uchar &= uchar` `cast[int](b)`; `i128 &= int` `cast[__int128](b)`; `int &= long long` bare long long: R is wider than T and nothing converts the lhs). `uac40.py`: all 588 rows satisfy "rhs effective type = UAC", and every node type is the lhs type (the only mismatches of a width-only model are `long` vs `long long`, same width, EDG uses the higher rank). This is the shape of the arithmetic compounds (`+= <<=`: `ref/ref.c`), so `assignment_parts` can take the same path. The conversion of the result back to T is not in the IL (a `bool` lhs: result `!= 0`; a narrower lhs: a value that may not fit is the 0022 `conversion` check). |
| value of a compound used (`return a &= b;`, `int r = a &= b;`) | the node as above (C++: an lvalue), with the use's own generated `cast[int]`/nothing around it (P33's shape). |

**2. Enumeration operands** (`probes/enum/e1.{c,cpp}`, `c.fn`, `cpp.fn`):

| shape | C23 | C++20 |
|---|---|---|
| `~c`, `c` an `enum color` (values 0..2, no negative enumerator) | `cast[int] {gen}(complement[enum color](var c))`: **bare enumeration operand, the node has the enumeration's type** (= `unsigned int`), the cast to int is outside (P26's note confirmed; also `enum neg`: `complement[enum neg]`) | `complement[int](cast[int] {gen}(var c))` |
| `c & 1`, `1 & c`, `c \| 1u`, `c ^ d`, `c \| f` (two enumerations), `(c & d) == RED` | computed in **`unsigned int`**, the enumeration operand cast to it (`and[unsigned](cast[unsigned](c), 1U)`, then `cast[int]`); `enum neg & 3`: `and[int](cast[int](n), 3)` | computed in `int` (`and[int](cast[int](c), 1)`), `c \| 1u`: `or[unsigned](cast[unsigned](c), 1U)` |
| `RED \| GREEN`, `RED \| NA`, `~RED` | folded: `K<int 1 expr=or[int](K<int 0 expr=cast[int] {gen}(K<int 0 enumerator>) orig_type=int>, K<int 1 expr=cast[int] {gen}(K<int 1 enumerator>) ...>)>`, i.e. the enumerators are int constants with a generated cast copy of the value, the inner constant is flagged `enumerator` | `or[int](K<int 0 expr=cast[int](K<color 0 enumerator>) orig_type=color>, ...)`; two different enumerations (`RED \| NA`) both fold, no error |
| `x &= RED`, `x & GREEN` | rhs `K<int 0 expr=cast[int] {gen}(K<int 0 expr=NONE enumerator>) orig_type=int>`: the unflagged copy of 0018 item 6 (flag on the inner constant only) | same with `K<color 0 enumerator>` |
| `c &= 1` | accepted by C (`and_assign[enum color](c, K<unsigned 1U expr=cast(1) orig_type=int>)`: **node type = the enumeration, rhs cast to unsigned**) | **error #511** (front end), exit 2 |
| `(c ? GREEN : BLUE) & 1` | `?:` is **int-typed**, arms `K<int 1 enumerator>` / `K<int 2 enumerator>`, condition bare `c` | `?:` has type `color`, arms `K<color 1 enumerator>`, condition `bool_cast(c)` |

**3. Folded bitwise constants** (`probes/fold/*.{c,cpp}` and `index.json`, generated by `gen40c.py`: 20 int-valued and 11 unsigned/wide expressions (`0xF0 | 0x0F`, `~0`, `~0u`, `-8 & 13`, `(1 << 4) | 1`, `2 ^ 32`, nested, mixed `~0 & 5 | 2 ^ 1`, `(0xF0 | 0x0F) == 255`, `(1 | 2) ? 7 : 8`, `~(-2147483647 - 1)`, `(unsigned char)~0`, `~0ull`, `0x7FFFFFFF | 0x80000000`, ...), **1312 probe lines** over 25 places in both modes). Two independent oracles:
- **encoder oracle** (`chk40.py`, `chk40.out`): today's `build/cerastes verify` rejects the bitwise operator at the line of **every** probe line (1312 of 1312, except C's `nnest.c`, 20 lines, which EDG's #1901 refuses before: a `const` object in a constant expression); `col40.py`: the caret is inside the *last written occurrence of E* (not the `__typeof__(E)` of the type), for every place but `opd` (the written `x & (E)`, not decisive) and `enum2` (the first rejected declaration of the line is `AB = A | B`); `enum3` (`X = E` first) confirms the enumerator. A folded constant without `expr` would verify silently with EDG's value, so this shows the encoder reaches an original tree in each place.
- **IL oracle** (`il40.py`): 6143 `K` nodes over the printable places: 2657 with `expr`, **0 with `expr=NONE` whose value is not a literal written on the line**.

| place | `expr` kept | note |
|---|---|---|
| return, initializer, assignment, argument, condition, local `const`, `x == (E)`, case value `case E:` (also over an unsigned `x`) | **kept** | e.g. `-8 & 13`: `K<int 8 expr=and[int](K<int -8 expr=negate(K<int 8>)>, K<int 13>)>` |
| file-scope `const`/`constexpr` initializer (`const T K = E;`, `const long long K = E`), use `K & 1`, `K \| 1` (C++; C is #1901) | **kept**; the use is `K<int 3 expr=var K>` | |
| enumerator initializers: `A = E`, `AB = A \| B`, `X = E, Y = X + 0` | **kept** (C: `K<int 3 expr=or[int](K<int 1 enumerator>, K<int 2 enumerator>) enumerator>`; C++: the constants are typed `K<e_b03 8 ...>` for earlier enumerators) | |
| C++ default argument `int f(int a = E)` | **kept** on the call's generated constant | |
| contract `requires`/`ensures` (`il_display` prints them) | `0xF0 \| 0x0F`, `-8 & 13` etc.: **unfolded** (`eok_or[int] {par}(240, 15)`); `~0`: folded with `expr=complement {par}` (`K<int -1 expr=complement[int] {par}(K<int 0>)>`); unsigned: gen casts around, `{par}` on the `and` | no folded constant without `expr` |
| `check`, `invariant`, `decreases` | not printed by `il_display` (M1); the encoder oracle rejects at the operator inside the attribute's tokens for all 31 + 31 + 20 lines | |

**4. `is_parenthesized`** (`probes/par/p1.*` 50 shapes, `p2.*` 20, `p*.sum`, `par` = the flag):
- It is set on **the node of the parenthesized expression**, never on a generated cast around it: `(a & b) == c` is `eq(and {par}(a, b), c)` and in C++ `cast[int] {gen}(eq[bool](and {par}(a, b), c))` (the cast has no flag); `(a & b) ? c : d` is `question(and {par}(a, b), ...)` (C++: `bool_cast {gen}` around the `and {par}`); `(x & 3) == (1 | 2)`: `eq(and {par}, K<3 expr=or {par}(1, 2)>)`. Same nodes in C23 and C++20 (the diff of 50 functions is generated casts only).
- **Folded constants:** the flag is on the **root of the constant's `expr` tree** (`x & (1 == 2)`: `K<int 0 expr=eq[int] {par}(1, 2)>`; `(1 | 2) & x`: `K<int 3 expr=or {par}(1, 2)>`; double parentheses `((1 | 2))` the same single flag; C++: `K<int 1 expr=cast[int] {gen}(K<bool 1 expr=eq[bool] {par}(1, 1)>)>`: the flag is on the innermost original operation under any chain of generated casts and `K`-with-`expr` wrappers). Never on the `enk_constant` node or the `a_constant` (0 occurrences in the 70 files).
- **Lost:** a parenthesized literal, named constant or enumerator (`(3) & x`, `(K) & x`, `(RED) | x`): the constant has no operation node to carry the flag (`K<int 3 expr=NONE>`, `K<int 3 expr=var K>` unflagged). A parenthesized variable keeps it (`(x) & (y)`: `var x {par,olt=int}`). Harmless for the `-Wparentheses` family (the shapes name an *operation* operand), recorded.
- Unparenthesized shapes carry no flag anywhere: `x & 1 == 2` is `and(x, K<int 0 expr=eq(1, 2)>)`, `a & b == c` is `and(a, eq(b, c))`, `a | b & c` is `or(a, and(b, c))`, `1 | 2 & 3` folds with the inner `and` unflagged, `1 == 1 | x`: `K<int 1 expr=eq>` unflagged.

**5. `?:`, comma, bool-valued shapes** (`probes/cnd/c1.*`, 34 shapes, `c1.*.tree`):
- **C23:** `x ? true : false` is `question[int](x, K<int 1 expr=cast[int] {gen}(K<bool 1>)>, K<int 0 expr=cast[int] {gen}(K<bool 0>)>)` (the arms are folded **with `expr`**: a generated cast of the `bool` constant); `x ? (x > 0) : (x < 0)` is int-typed with `gt[int] {par}`/`lt[int] {par}` arms; `b ? b : b` (bool b) is `bool_cast[bool](question[int](b, cast[int](b), cast[int](b)))` (the condition is bare, the arms cast to int); `x ? y : 0` bare.
- `c == (x++, x > 0)` (`enum color c`): `eq[int](cast[unsigned] {gen}(c), cast[unsigned] {gen}(comma[int] {par}(post_incr {nu}(x), gt[int](x, 0))))` in C, `eq[bool](cast[int](c), cast[int](comma[bool] {par}(...)))` in C++. The comma carries `{par}`; its left operand is `{nu}` (and `{lv,nu}` for a bare `x` in C++).
- **Enumerators in a C `?:`:** `c == (x ? RED : GREEN)`: `?:` is `int`-typed `{par}`, the arms are **`K<int 0 enumerator>`, `K<int 1 enumerator>`** (flagged, as 0018 describes). When the whole `?:` is folded (`(1 ? NB : NC)`, `n == (0 ? NB : NC)`) the constant is `K<int 0 expr=cast[int] {gen}(K<int 0 expr=question[int] {par}(K<int 1>, K<int 0 enumerator>, K<int 1 enumerator>)>)>`: the arms keep the flag inside the `expr`. Where the arm is converted (a mixed `x ? RED : x`, `x ? NA : 1`) the arm is the **unflagged copy** `K<int 0 expr=cast[int] {gen}(K<int 0 enumerator>)>` (the flag is on the inner constant only), and `NA = -1` is `K<int -1 expr=negate[int](K<int 1>) enumerator>`. `c == (x, RED)`, `c == (0, RED)`: the comma's right operand `K<int 0 enumerator>` (folded `(0, RED)` is `K<unsigned 0U expr=cast[unsigned](K<int 0 enumerator>)>`).
- **C++:** the same `?:` has the enumeration type (`question[color](bool_cast(x), K<color 0 enumerator>, K<color 1 enumerator>)`, folded: `K<color 0 expr=question[color] {par}(K<bool 1 expr=bool_cast(K<int 1>)>, K<color 0 enumerator>, ...)>`); a mixed one (`x ? RED : x`) is int-typed with the unflagged copy as arm; `x ? true : false` is `question[bool]` over bare `K<bool 1>`.

### Q33: character literals (`probes/chr2/q33.txt`, raw stderr `<f>.<mode>.{plain,err}.txt`; `--display_error_number`; the second run adds `--diag_error=1422,1654,26,3255`; `--il_display` accepted, the folded value is not shown)

Tag names (the harness's `TagRegistry`, `tags40.py`): **1422 `multi_char_literal`, 1654 `leading_character_ignored_in_char_literal`, 26 `too_many_characters`, 3255 `utf16_char_lit_too_long`**; also hard errors 1535 `no_char16_t_representation`, 25 `zero_length_string`, 27 `bad_character_value`, 3439 `char_too_wide_for_rep`.

| literal | C23 plain | C++20 plain | with `--diag_error=1422,1654,26,3255` |
|---|---|---|---|
| `'ab'`, `'abc'`, `'abcd'`, `'\x41\x42'` | warning #1422-D, exit 0 | same | error #1422-D, exit 2, both modes |
| `'abcde'` | warning #1654-D | same | error #1654-D, exit 2 |
| `L'ab'`, `L'abc'`, `u'ab'`, `U'ab'`, `u'éa'` | warning **#26-D** | **hard error #26** (exit 2, no option needed) | C: error #26-D, exit 2; C++: unchanged |
| `u8'ab'`, `u8'\U0001F600'` | warning **#1422-D** | warning **#26-D** | error in both, exit 2 |
| `u'\U0001F600'` | warning #3255-D | **hard error #1535** | C: error #3255-D, exit 2 |
| `U'\U0001F600'`, `L'\U0001F600'`, `L'é'`, `u8'a'`, `u8'\xff'` | no diagnostic | no diagnostic | exit 0 |
| `'a'`, `'\n'`, `'\101'`, `'\x41'`, `'\xff'` | no diagnostic | no diagnostic | exit 0 |
| `'\x141'` | hard error #27-D (exit 2) | same | unchanged |
| `''` | hard error #25 | same | unchanged |
| `'é'` | hard error #3439-D | same | unchanged |

Today's `build/cerastes verify` accepts `'ab'`, `'abcde'` and `u'\U0001F600'` (C23) with EDG's warning and result `verified`; the `--diag_error` list above turns every one of them into a front-end error in both modes (so `rejected multi_char_literal @ L<n>` etc.). (A case-insensitive file system collapsed `uab`/`Uab` in the first run; `probes/chr/` is stale, `probes/chr2/` is the result.)

### Implications for `encode.cpp`

- `operation()` gets `eok_and`, `eok_or`, `eok_xor`, `eok_complement`: operands arrive **already converted to the result type** by generated casts (the existing cast path handles them, including `bool` operands cast to `int`); the node's type is R, no recomputation. `eok_complement` on a **bare enumeration operand in C** (type `enum color`, result type the enumeration) must be handled before the generated `cast[int]` that follows: its value is `~` of the enumeration's base (`unsigned` for `color`, `int` for `neg`), and the cast to `int` is a `conversion` check.
- Compound forms: `eok_and_assign|or_assign|xor_assign`: node type = lhs type, lhs bare, rhs already cast to R; compute in R = **the rhs operand's own (cast) type**, then convert to T (the 0022 checked conversion; `bool` lhs: `!= 0`). `assignment_parts` can add the three kinds beside the arithmetic ones; the existing `is_enum_type(lhs)` rejection already covers `c &= 1` in C (C++ is an EDG error).
- Folded constants need nothing beyond what 0010 item 12 / 0022 item 3 already do: P40 confirms `expr` everywhere. Read the written tree through chains of `K`-with-`expr` and generated casts; the **parenthesis flag of a folded constant is on the innermost original operation** (`is_parenthesized` of the `expr` root after peeling generated casts and `K` wrappers), never on the constant.
- Warning shapes (`-Wparentheses`, `-Wbitwise-op-parentheses`, `-Wbitwise-conditional-parentheses`): the flag is on the operand operation node in both modes (C++: under no generated cast, the casts are outside); look through a `K` with `expr` for folded operands; literals, named constants and enumerators never carry it.
- Enumeration C/C++ difference: `c & 1` is `unsigned` in C and `int` in C++ for an enumeration without negative enumerators; `~c` differs in value (and the `int` cast of `~c` in C is a `conversion` obligation).
- Q33: `--diag_error=1422,1654,26,3255` (add `3255` in C only matters) suffices in both modes; the harness tags are the four above.

### P41: which bitwise-operator shapes the native compilers and EDG warn on (decision 0023): **PASS (matrix below; 7 new rules and 2 extensions, the union is stated)**

*Names (added with record 0023):* P41's rules 1–7 are record 0023 item 5's tags `bitwise_operand_parentheses` (1),
`logical_not_bitwise` (2), `complement_of_truth_value` (3), `bitwise_condition_parentheses` (4),
`bitwise_instead_of_logical` (5), `tautological_bitwise_compare` (6a, 6b and, after audit round 1, 6c) and
`xor_as_power` (7). The tables below give shapes without their operand types where the type is `int`; a shape whose
verdict depends on the type (`7a` against `b40`, `b4`) says so in the text of its rule. Audit round 1 also found clang's
`-Wtautological-constant-out-of-range-compare` on `(p ^ q) == 2` and GCC's `-Wtype-limits` on `(uc & vc) == 300`,
which this matrix did not probe (record 0023 item 5, "Not rejected, listed").

Evidence is under `build/m4.4b1/p41/` (gitignored): `shapes*.py` (the shape lists), `gen.py` (one function per source line, `p41*.c` and `p41*.cpp`; `p41*.json` maps a line to its shape), `matrix.py` (the runs), `run.sh` (regenerates everything, about 4 minutes), `raw/` (every
compiler output), `p41*.matrix.txt|json` and `p41*.detail.json` (cells and messages), `sweep.py` (the xor sweep), `q33/` (the header sweep), `mkdoc.py` + `P41.tmpl.md` (this text).
Apple clang 21.0.0 (`clang-2100.3.34.2`, Command Line Tools) and GCC 16.2.0 (Homebrew `gcc-16`/`g++-16`), `-fsyntax-only` with `-std=c23` in C and `-std=c++20` in C++, each shape compiled four times (no `-W` flag, `-Wall`, `-Wextra`, `-Wall -Wextra`);
EDG is `scripts/cerastes-fe.sh --c23|--c++20 --display_error_number` (EDG's #550/#551 "set but not used" left out, as are the compilers' `-Wunused-parameter`/`-Wunused-variable`/`-Wunused-but-set-*`). 916 shapes in five files:
`p41` (315: the eleven groups of the plan), `p41b` (505: sweeps and variants), `p41c` (30), `p41d` (26), `p41e` (40). A cell is the warning flag, `-` for none; no suffix means `-Wall` enables it, `[E]` that only `-Wextra` does, `[D]` that it is on without any flag.
`-Wsign-compare` is `[E]` in C and plain (in `-Wall`) in C++ for GCC. All flags below are in `-Wall` unless a suffix says otherwise. A native-ok test uses both flags with `-Werror`, so every cell that is not `-` fails it.

**EDG warns on none of the bitwise shapes** (every EDG cell is `-`). Its only entries in these files are unrelated: #174 (a comma operand without effect: `~(a, 1)`, `x &= (0, x)`), #186 (`u >= 0`, `u < 0`), #68 (`(int)(u & -1)`). So every rule below is cerastes's own (as 0020 item 5).
**Contract expressions are invisible to the compilers** (`CERASTES_REQUIRES(...)` expands to nothing, `<cerastes.h>`), so the same shapes in a `requires`/`ensures` get no warning from anyone; `--cerastes_walk` shows them as ordinary trees (`a & b == c` is `eok_and` over `a` and an implicit `eok_cast` int of `eok_eq`;
`contract.cpp`). The compilers do warn in `static_assert` and enumerator values (`k1`, `k2`, `k6`, `k7`: same cells as code). Applying the rules to contracts too is the uniform, stricter choice (recommendation; 0023's decision).

Peeling used below: parentheses are the flag `is_parenthesized` (no node), the front end's promotions and conversions are `compiler_generated` casts (peeled), an explicit cast is a node without that flag (never peeled except where stated). A **truth value** T is, after that peeling:
a comparison (`== != < > <= >=`), `!e`, `&&`, `||`, a `bool`-typed expression (variable, call, `(bool)` cast, `true`/`false`, a `?:` of bool arms).

**1. A comparison, `+`/`-` or another bitwise operator as an operand of `& | ^`** (`-Wparentheses`; clang's `-Wbitwise-op-parentheses` for the third)

| id | shape | clang C | clang C++ | gcc C | gcc C++ | EDG (C / C++) |
|---|---|---|---|---|---|---|
| 1a | `a & b == c;` | parentheses[D] | parentheses[D] | parentheses | parentheses | - / - |
| 1b | `a == b & c;` | parentheses[D] | parentheses[D] | parentheses | parentheses | - / - |
| 1e | `(a == b) & (c == d);` | - | - | - | - | - / - |
| 1o | `(a == b) & c;` | - | - | - | - | - / - |
| 1p | `a & (b == c);` | - | - | - | - | - / - |
| 1q | `a == b & c == d;` | - | - | parentheses | parentheses | - / - |
| 1r | `CMPAND(a, b, c);` | parentheses[D] | parentheses[D] | - | - | - / - |
| 1s | `PARAND(a, b, c);` | parentheses[D] | parentheses[D] | - | - | - / - |
| 1t | `5 & 3 == 3;` | parentheses[D] | parentheses[D] | parentheses | parentheses | - / - |
| 1u | `a & b == 0 ? 1 : 2;` | bitwise-conditional-parentheses[D],parentheses[D] | bitwise-conditional-parentheses[D],parentheses[D] | parentheses | parentheses | - / - |
| 1B | `a \| b != c;` | parentheses[D] | parentheses[D] | parentheses | parentheses | - / - |
| 1E | `a & b == c & d;` | parentheses[D] | parentheses[D] | parentheses | parentheses | - / - |
| 1G | `CMPRET(a, b, c);` | bitwise-conditional-parentheses[D],parentheses[D] | bitwise-conditional-parentheses[D],parentheses[D] | - | - | - / - |
| 1v | `(a < b) \| (c < d);` | - | - | - | - | - / - |
| 1z | `(a & b) == c;` | - | - | - | - | - / - |
| 2a | `a \| b + c;` | - | - | parentheses | parentheses | - / - |
| 2b | `a & b - c;` | - | - | parentheses | parentheses | - / - |
| 2c | `a ^ b + c;` | - | - | parentheses | parentheses | - / - |
| 2d | `a + b \| c;` | - | - | parentheses | parentheses | - / - |
| 2e | `a - b & c;` | - | - | parentheses | parentheses | - / - |
| 2f | `a \| b & c;` | bitwise-op-parentheses | bitwise-op-parentheses | parentheses | parentheses | - / - |
| 2g | `a ^ b & c;` | bitwise-op-parentheses | bitwise-op-parentheses | parentheses | parentheses | - / - |
| 2h | `a \| b ^ c;` | bitwise-op-parentheses | bitwise-op-parentheses | parentheses | parentheses | - / - |
| 2i | `a & b \| c;` | bitwise-op-parentheses | bitwise-op-parentheses | parentheses | parentheses | - / - |
| 2j | `a ^ b \| c;` | bitwise-op-parentheses | bitwise-op-parentheses | parentheses | parentheses | - / - |
| 2k | `a & b ^ c;` | bitwise-op-parentheses | bitwise-op-parentheses | parentheses | parentheses | - / - |
| 2l | `(a & b) \| c;` | - | - | - | - | - / - |
| 2n | `a \| b \| c;` | - | - | - | - | - / - |
| 2o | `a & b & c;` | - | - | - | - | - / - |
| 2p | `a ^ b ^ c;` | - | - | - | - | - / - |
| 2q | `a \| b * c;` | - | - | - | - | - / - |
| 2r | `a << b \| c;` | - | - | - | - | - / - |
| 2s | `a \| b << c;` | - | - | - | - | - / - |
| 2t | `u + v & w;` | - | - | parentheses | parentheses | - / - |
| 2u | `MIXOR(a, b, c);` | - | - | - | - | - / - |
| 2v | `uc \| vc + 1;` | - | - | parentheses | parentheses | - / - |
| 2w | `a \| b / c;` | - | - | - | - | - / - |
| 2F | `a ^ b & c \| d;` | bitwise-op-parentheses | bitwise-op-parentheses | parentheses | parentheses | - / - |
| 2G | `p \| q & r;` | bitwise-op-parentheses | bitwise-op-parentheses | parentheses | parentheses | - / - |
| 2I | `a & b ? c \| d : 0;` | - | - | - | - | - / - |

Both compilers warn on every unparenthesized comparison operand of `&`, `|`, `^` (left or right, any type: `int`, `unsigned`, `bool`, `char`, constants, in a return, a condition, an initializer, `static_assert`, an enumerator); **clang is silent on `a == b & c == d`** (`1q`, both operands comparisons), GCC is not.
**Only GCC** warns on `+`/`-` as an operand (`2a`-`2e`, `2t`, `2v`, `2A`, any type). **Both** warn on a bitwise operator under a different one (`&` under `|` or `^`, `^` under `|`, on either side: `2f`-`2k`, `2F`-`2H`); a chain of one operator (`2n`-`2p`), shifts (`2r`, `2s`, `2y`, `2z`), `*`, `/`, `%`
and unary `-`, `+`, `~`, `!` are silent. Parenthesized operands are silent in all four. **Macros:** clang warns in a macro body from the same file (`1r`, `1s`, `1G`, `m3`, `m4`, `b45`); GCC is silent for `1r`, `1s`, `m3` and, in C, for `m1`/`b45` (it warns for those two in C++);
neither warns on `+` in a macro (`2u`). The IL cannot tell the cases apart reliably, so the rule rejects regardless of macros.

**2. `!x` on the left of `&` or `|`**

| id | shape | clang C | clang C++ | gcc C | gcc C++ | EDG (C / C++) |
|---|---|---|---|---|---|---|
| n1 | `!x & y;` | logical-not-parentheses[D] | logical-not-parentheses[D] | parentheses | parentheses | - / - |
| n2 | `!x \| y;` | - | - | parentheses | parentheses | - / - |
| n3 | `!x & 1;` | logical-not-parentheses[D] | logical-not-parentheses[D] | - | - | - / - |
| n4 | `!x \| 1;` | - | - | - | - | - / - |
| n5 | `!x & 2;` | logical-not-parentheses[D] | logical-not-parentheses[D] | parentheses | parentheses | - / - |
| n8 | `!x & (y == 0);` | - | - | - | - | - / - |
| n9 | `!x & !y;` | - | - | - | - | - / - |
| n11 | `!x & q;` | - | - | - | - | - / - |
| n12 | `!p & y;` | - | - | parentheses | parentheses | - / - |
| n13 | `!x & (int)(y > 0);` | logical-not-parentheses[D] | logical-not-parentheses[D] | - | - | - / - |
| n15 | `!!x & y;` | - | - | parentheses | parentheses | - / - |
| n16 | `!x & y & z;` | logical-not-parentheses[D] | logical-not-parentheses[D] | parentheses | parentheses | - / - |
| n17 | `!x & !y & z;` | - | - | - | - | - / - |
| n19 | `!(x > 0) & y;` | - | - | parentheses | parentheses | - / - |
| n21 | `!x ^ y;` | - | - | - | - | - / - |
| n22 | `(!x) \| y;` | - | - | - | - | - / - |
| n24 | `!x \| (y > 0);` | - | - | - | - | - / - |
| n30 | `!x & 0;` | logical-not-parentheses[D] | logical-not-parentheses[D] | - | - | - / - |
| n31 | `!x \| 0;` | - | - | - | - | - / - |
| n35 | `!uc & y;` | logical-not-parentheses[D] | logical-not-parentheses[D] | parentheses | parentheses | - / - |
| n37 | `!x \| -y;` | - | - | parentheses | parentheses | - / - |
| and_castq | `!x & (int)q;` | logical-not-parentheses[D] | logical-not-parentheses[D] | parentheses | parentheses | - / - |
| and_castcmp | `!x & (int)(y > 0);` | logical-not-parentheses[D] | logical-not-parentheses[D] | - | - | - / - |
| and_boolcall | `!x & fb(y);` | - | - | - | - | - / - |
| and_boolcast | `!x & (bool)y;` | - | - | - | - | - / - |
| and_true | `!x & true;` | - | - | - | - | - / - |
| and_one_l | `!x & 1L;` | logical-not-parentheses[D] | logical-not-parentheses[D] | - | - | - / - |
| or_castq | `!x \| (int)q;` | - | - | parentheses | parentheses | - / - |
| or_castcmp | `!x \| (int)(y > 0);` | - | - | - | - | - / - |
| or_two | `!x \| 2;` | - | - | parentheses | parentheses | - / - |
| or_m1 | `!x \| -1;` | - | - | parentheses | parentheses | - / - |
| or_charlit | `!x \| 'a';` | - | - | parentheses | parentheses | - / - |
| or_one_l | `!x \| 1L;` | - | - | - | - | - / - |
| or_one | `!x \| 1;` | - | - | - | - | - / - |
| or_zero | `!x \| 0;` | - | - | - | - | - / - |
| or_boolcall | `!x \| fb(y);` | - | - | - | - | - / - |
| or_true | `!x \| true;` | - | - | - | - | - / - |

Clang (`-Wlogical-not-parentheses`, on by default) warns on `!e & r` only, never on `|` or `^`, for every `r` that is not a truth value, **including literals** (`!x & 1`, `!x & 0`, and `(int)(y > 0)`, `(int)q`); GCC (`-Wparentheses`) warns on `!e & r` and `!e | r` (not `^`) unless `r` is a truth value by form (a comparison, `!`, `&&`,
`||`, a bool variable or call, a `(bool)` cast, `(int)(y > 0)`) **or the literal 0 or 1** (`!x & 1`, `!x | 1`, `!x | 0`: silent). GCC warns when `e` is itself a truth value (`!!x & y`, `!(x > 0) & y`, `!p & y`), clang does not. `!` as the right operand (`x & !y`, `y | !x`), `!x ^ y`, `!x & !y`, `!x & !y & z` (the inner `&` has a truth right operand), `(!x) | y`
are silent. Types do not matter (`!uc & y`, `!u & v` warn).

**3. `?:` with a bitwise condition** (clang `-Wbitwise-conditional-parentheses`, on by default; GCC never)

| id | shape | clang C | clang C++ | gcc C | gcc C++ | EDG (C / C++) |
|---|---|---|---|---|---|---|
| 4a | `a & b ? c : d;` | - | - | - | - | - / - |
| 4b | `a \| b ? c : d;` | - | - | - | - | - / - |
| 4c | `a ^ b ? c : d;` | - | - | - | - | - / - |
| 4d | `a & 1 ? b : c;` | - | - | - | - | - / - |
| 4e | `(a == b) & c ? 1 : 2;` | - | - | - | - | - / - |
| 4f | `(a & b) ? c : d;` | - | - | - | - | - / - |
| 4g | `a ? b & c : d;` | - | - | - | - | - / - |
| 4i | `a \| (b > 0) ? c : d;` | bitwise-conditional-parentheses[D] | bitwise-conditional-parentheses[D] | - | - | - / - |
| 4j | `p & q ? 1 : 2;` | bitwise-conditional-parentheses[D] | bitwise-conditional-parentheses[D] | - | - | - / - |
| 4n | `a & b == c ? 1 : 2;` | bitwise-conditional-parentheses[D],parentheses[D] | bitwise-conditional-parentheses[D],parentheses[D] | parentheses | parentheses | - / - |
| q1 | `a \| (b > 0) ? c : d;` | bitwise-conditional-parentheses[D] | bitwise-conditional-parentheses[D] | - | - | - / - |
| q2 | `(b > 0) \| a ? c : d;` | - | - | - | - | - / - |
| q3 | `a & !b ? c : d;` | bitwise-conditional-parentheses[D] | bitwise-conditional-parentheses[D] | - | - | - / - |
| q5 | `a & (b && c) ? c : d;` | bitwise-conditional-parentheses[D] | bitwise-conditional-parentheses[D] | - | - | - / - |
| q6 | `a & q ? c : d;` | bitwise-conditional-parentheses[D] | bitwise-conditional-parentheses[D] | - | - | - / - |
| q7 | `p & b ? c : d;` | - | - | - | - | - / - |
| q8 | `p \| q ? c : d;` | bitwise-conditional-parentheses[D] | bitwise-conditional-parentheses[D] | - | - | - / - |
| q9 | `a ^ q ? c : d;` | - | - | - | - | - / - |
| q10 | `a ^ (b > 0) ? c : d;` | - | - | - | - | - / - |
| q11 | `a & (b > 0) ? c : d;` | bitwise-conditional-parentheses[D] | bitwise-conditional-parentheses[D] | - | - | - / - |
| q12 | `(a > 0) & (b > 0) ? c : d;` | bitwise-conditional-parentheses[D] | bitwise-conditional-parentheses[D] | - | - | - / - |
| q13 | `(a > 0) \| b ? c : d;` | - | - | - | - | - / - |
| q15 | `a \| (int)(b > 0) ? c : d;` | - | - | - | - | - / - |
| q16 | `a \| (b > 0 ? 1 : 2) ? c : d;` | - | - | - | - | - / - |
| q17 | `a \| fb(b) ? c : d;` | bitwise-conditional-parentheses[D] | bitwise-conditional-parentheses[D] | - | - | - / - |
| q18 | `fb(a) \| b ? c : d;` | - | - | - | - | - / - |
| q20 | `(a \| (b > 0)) ? c : d;` | - | - | - | - | - / - |
| q22 | `a & (b \|\| c) ? c : d;` | bitwise-conditional-parentheses[D] | bitwise-conditional-parentheses[D] | - | - | - / - |
| q23 | `a \| !b ? c : d;` | bitwise-conditional-parentheses[D] | bitwise-conditional-parentheses[D] | - | - | - / - |
| q24 | `a \| ~b ? c : d;` | - | - | - | - | - / - |

Clang warns when the unparenthesized condition is `&` or `|` (not `^`) and its **right** operand is a truth value (comparison, `!`, `&&`, `||`, bool variable or call), also when the left operand is a truth value too (`q12`); it is silent when only the left operand is a truth value (`q2`, `q7`, `q13`, `q18`), for an explicit `(int)` cast (`q15`),
`~b`, a `?:`, a parenthesized condition (`q20`), and for plain `a & b ? c : d`, `a | b ? c : d`, `a & 1 ? ...` (`4a`-`4d`). Nothing else is warned about `a & b ? c : d` (the plan's guess was wrong: no compiler warns there).

**4. `~` of a truth value** (`-Wbool-operation`, in `-Wall`)

| id | shape | clang C | clang C++ | gcc C | gcc C++ | EDG (C / C++) |
|---|---|---|---|---|---|---|
| 5a | `~p;` | bool-operation | bool-operation | bool-operation | bool-operation | - / - |
| 5b | `~(x > 0);` | bool-operation | bool-operation | bool-operation | bool-operation | - / - |
| 5c | `~!x;` | bool-operation | bool-operation | bool-operation | bool-operation | - / - |
| 5d | `~0;` | - | - | - | - | - / - |
| 5e | `~1;` | - | - | - | - | - / - |
| 5f | `~true;` | bool-operation | bool-operation | bool-operation | bool-operation | - / - |
| 5g | `~x;` | - | - | - | - | - / - |
| 5h | `~(p && q);` | bool-operation | bool-operation | bool-operation | bool-operation | - / - |
| 5m | `~(p == q);` | bool-operation | bool-operation | bool-operation | bool-operation | - / - |
| 5n | `~(a & b);` | - | - | - | - | - / - |
| 5o | `~(p & q);` | bool-operation | bool-operation | - | - | - / - |
| 5p | `~(int)p;` | - | - | - | - | - / - |
| 5w | `~(p ^ q);` | bool-operation | bool-operation | - | - | - / - |
| 5x | `~false;` | bool-operation | bool-operation | bool-operation | bool-operation | - / - |
| t1 | `~(p ? q : r);` | bool-operation | bool-operation | - | bool-operation | - / - |
| t2 | `~fb(a);` | bool-operation | bool-operation | bool-operation | bool-operation | - / - |
| t3 | `~(bool)a;` | bool-operation | bool-operation | bool-operation | bool-operation | - / - |
| t4 | `~(a > 0 ? 1 : 2);` | - | - | - | - | - / - |
| t5 | `~(p \| q);` | bool-operation | bool-operation | - | - | - / - |
| t6 | `~(p & fb(a));` | bool-operation | bool-operation | - | - | - / - |
| t7 | `~((a > 0) & (b > 0));` | bool-operation | bool-operation | - | - | - / - |
| t8 | `~((a > 0) \| b);` | - | - | - | - | - / - |
| t9 | `~(p ^ true);` | bool-operation | bool-operation | - | - | - / - |
| t10 | `~((a > 0) \| (b > 0));` | bool-operation | bool-operation | - | - | - / - |
| t12 | `~(a & 1);` | - | - | - | - | - / - |
| t14 | `~(!a);` | bool-operation | bool-operation | bool-operation | bool-operation | - / - |
| t16 | `~(a ? 1 : 0);` | - | - | - | - | - / - |
| t17 | `~(p ? a : 1);` | - | - | - | - | - / - |
| t18 | `p = ~p; return p;` | bool-operation | bool-operation | bool-operation | bool-operation | - / - |
| t20 | `~(unsigned)p;` | - | - | - | - | - / - |
| t21 | `~+p;` | bool-operation | bool-operation | - | - | - / - |
| t22 | `~-p;` | - | - | - | - | - / - |
| t23 | `~~p;` | bool-operation | bool-operation | bool-operation | bool-operation | - / - |
| t25 | `~(a & q);` | - | - | - | - | - / - |
| t32 | `~(1 == 1);` | bool-operation | bool-operation | - | bool-operation | - / - |
| t33 | `~(0 \|\| 1);` | bool-operation | bool-operation,constant-logical-operand[D] | - | bool-operation | - / - |
| k4 | `static_assert(~true != 0, "");` | bool-operation | bool-operation | bool-operation | bool-operation | - / - |
| c10 | `~p == 1;` | bool-operation | bool-operation | bool-operation | bool-operation | - / - |

Both compilers warn on `~` of: a `bool` variable (`~p`, `p = ~p`, `~~p` once), a call returning `bool`, `(bool)a`, `!a`, a comparison, `&&`, `||`, `true`, `false`. **Clang (C and C++) and g++ also** warn on `~(p ? q : r)` and on `~(1 == 1)`/`~(0 || 1)` (GCC C is silent); **clang only** on `~+p`
and on `&`, `|`, `^` whose **both** operands are truth values (`t5`-`t7`, `t9`-`t11`, `5o`, `5w`; `~(a & b)`, `~(a > 0) | b` (`t8`), `~(a & q)` (`t25`) are silent). Silent for everyone: `~0`, `~1` (`5d`, `5e`), `~x`, `~(a & 1)`, `~(a ? 1 : 0)` (`t16`), `~-p`, an explicit `(int)`
or `(unsigned)` cast of a truth value (`5p`, `t20`).

**5. Bitwise on truth values** (clang `-Wbitwise-instead-of-logical`; GCC never)

| id | shape | clang C | clang C++ | gcc C | gcc C++ | EDG (C / C++) |
|---|---|---|---|---|---|---|
| 6a | `p & q;` | - | - | - | - | - / - |
| 6b | `p \| q;` | - | - | - | - | - / - |
| 6c | `p ^ q;` | - | - | - | - | - / - |
| 6d | `p & fb(a);` | - | - | - | - | - / - |
| 6g | `a & b;` | - | - | - | - | - / - |
| 6h | `(a > 0) & (b > 0);` | - | - | - | - | - / - |
| e1 | `fb(a) \| gb(b);` | bitwise-instead-of-logical | bitwise-instead-of-logical | - | - | - / - |
| e2 | `fb(a) \| (b > 0);` | - | - | - | - | - / - |
| e3 | `(a > 0) \| gb(b);` | - | - | - | - | - / - |
| e5 | `p \| fb(a);` | - | - | - | - | - / - |
| e6 | `fb(a) \| p;` | - | - | - | - | - / - |
| e7 | `fi(a) \| fi(b);` | - | - | - | - | - / - |
| e8 | `(fi(a) > 0) \| (fi(b) > 0);` | bitwise-instead-of-logical | bitwise-instead-of-logical | - | - | - / - |
| e9 | `(fi(a) > 0) \| fb(b);` | bitwise-instead-of-logical | bitwise-instead-of-logical | - | - | - / - |
| e10 | `fb(a) \| (fi(b) > 0);` | bitwise-instead-of-logical | bitwise-instead-of-logical | - | - | - / - |
| e11 | `fb(a) \| gb(b) \| fb(c);` | bitwise-instead-of-logical | bitwise-instead-of-logical | - | - | - / - |
| e13 | `fb(a) ^ gb(b);` | - | - | - | - | - / - |
| e14 | `fb(a) & gb(b);` | bitwise-instead-of-logical | bitwise-instead-of-logical | - | - | - / - |
| e15 | `(fb(a)) \| (gb(b));` | bitwise-instead-of-logical | bitwise-instead-of-logical | - | - | - / - |
| e16 | `!fb(a) \| gb(b);` | bitwise-instead-of-logical | bitwise-instead-of-logical | - | - | - / - |
| e17 | `fb(a) \| !gb(b);` | bitwise-instead-of-logical | bitwise-instead-of-logical | - | - | - / - |
| e18 | `(p && q) \| fb(a);` | - | - | - | - | - / - |
| e19 | `fb(a) \| (bool)b;` | - | - | - | - | - / - |
| e22 | `bool t = true; t \|= fb(a); return t;` | - | - | - | - | - / - |
| e25 | `(int)fb(a) \| (int)gb(b);` | - | - | - | - | - / - |
| e27 | `!p \| fb(a);` | - | - | - | - | - / - |
| e28 | `(a > 0) & fb(b);` | - | - | - | - | - / - |
| e29 | `fb(a) & (b > 0);` | - | - | - | - | - / - |
| e30 | `((a > 0) \| (b > 0)) \| fb(a);` | - | - | - | - | - / - |
| w1 | `(x++ > 0) \| (y++ > 0);` | bitwise-instead-of-logical | bitwise-instead-of-logical | - | - | - / - |
| w2 | `(x++ > 0) \| (y > 0);` | - | - | - | - | - / - |
| w3 | `(x > 0) \| (y++ > 0);` | - | - | - | - | - / - |
| w4 | `p \| (y++ > 0);` | - | - | - | - | - / - |
| w5 | `fb(a) \| (y++ > 0);` | bitwise-instead-of-logical | bitwise-instead-of-logical | - | - | - / - |
| w6 | `(y++ > 0) \| fb(a);` | bitwise-instead-of-logical | bitwise-instead-of-logical | - | - | - / - |
| w8 | `(fb(a) && gb(b)) \| fb(c);` | bitwise-instead-of-logical | bitwise-instead-of-logical | - | - | - / - |
| w9 | `(fb(a) \|\| gb(b)) & fb(c);` | bitwise-instead-of-logical | bitwise-instead-of-logical | - | - | - / - |
| w10 | `fb(a) \| fb(b);` | bitwise-instead-of-logical | bitwise-instead-of-logical | - | - | - / - |
| w12 | `(a > fi(b)) \| (c > fi(d));` | bitwise-instead-of-logical | bitwise-instead-of-logical | - | - | - / - |
| w13 | `(fi(a) > 0) \| (b > 0);` | - | - | - | - | - / - |
| w14 | `(a > 0) \| (fi(b) > 0);` | - | - | - | - | - / - |
| w15 | `(fi(a) > 0) & (fi(b) > 0);` | bitwise-instead-of-logical | bitwise-instead-of-logical | - | - | - / - |
| w16 | `(x++ > 0) \| fb(y);` | bitwise-instead-of-logical | bitwise-instead-of-logical | - | - | - / - |
| w17 | `fb(y) \| (x++ > 0);` | bitwise-instead-of-logical | bitwise-instead-of-logical | - | - | - / - |
| w18 | `(x = 1) \| (y = 1);` | - | - | - | - | - / - |
| w20 | `(x++ > 0) & (y++ > 0);` | bitwise-instead-of-logical | bitwise-instead-of-logical | - | - | - / - |
| w21 | `(x++ > 0) ^ (y++ > 0);` | - | - | - | - | - / - |
| w24 | `int t = fb(a) \| gb(b); return t;` | bitwise-instead-of-logical | bitwise-instead-of-logical | - | - | - / - |
| w25 | `fb(a) \| gb(b) ? 1 : 0;` | bitwise-conditional-parentheses[D],bitwise-instead-of-logical | bitwise-conditional-parentheses[D],bitwise-instead-of-logical | - | - | - / - |
| w26 | `fb(a) \| (gb(b) \| fb(c));` | bitwise-instead-of-logical | bitwise-instead-of-logical | - | - | - / - |
| w27 | `(void)0, fb(a) \| gb(b);` | bitwise-instead-of-logical | bitwise-instead-of-logical | - | - | - / - |
| w28 | `(p ? p : q) \| fb(1);` | - | - | - | - | - / - |
| w29 | `(bool)fi(a) \| (bool)fi(b);` | bitwise-instead-of-logical | bitwise-instead-of-logical | - | - | - / - |
| w30 | `fi(a) != 0 \| fi(b) != 0;` | bitwise-instead-of-logical | bitwise-instead-of-logical | parentheses | parentheses | - / - |

Clang warns on `&` or `|` (never `^`, never `|=`/`&=`) when **both** operands are truth values **and both contain a side effect**: `fb(a) | gb(b)`, `(x++ > 0) | (y++ > 0)`, `!f() | !g()`, `(f() && g()) | h()`, `(fi(a) > 0) | (fi(b) > 0)`, `(bool)fi(a) | (bool)fi(b)`. One side effect is silent
(`p | fb(a)`, `(a > 0) | gb(b)`, `fb(a) | (b > 0)`, `(x++ > 0) | (y > 0)`), so are `int` operands (`fi(a) | fi(b)`), explicit `(int)` casts (`e25`) and pure truth values (`6a`, `6h`, `e4`). Only a call and `++` were probed as the effect.
`w30` (`fi(a) != 0 | fi(b) != 0`) is also rule 1 for GCC.

**6. Tautological comparisons and `~` of a narrow unsigned**

| id | shape | clang C | clang C++ | gcc C | gcc C++ | EDG (C / C++) |
|---|---|---|---|---|---|---|
| 7a | `(x & 2) == 1;` | taut-bitwise-compare | taut-bitwise-compare | tautological-compare | tautological-compare | - / - |
| 7b | `(x & 4) == 3;` | taut-bitwise-compare | taut-bitwise-compare | tautological-compare | tautological-compare | - / - |
| 7c | `(x \| 4) == 3;` | taut-bitwise-compare | taut-bitwise-compare | tautological-compare | tautological-compare | - / - |
| 7d | `(x & 2) != 1;` | taut-bitwise-compare | taut-bitwise-compare | tautological-compare | tautological-compare | - / - |
| 7e | `if (x \| 4) return 1; return 0;` | taut-bitwise-compare | taut-bitwise-compare | - | - | - / - |
| 7f | `if (x & 0) return 1; return 0;` | - | - | - | - | - / - |
| 7g | `(x & 4) == 4;` | - | - | - | - | - / - |
| 7h | `(x & 4) == 0;` | - | - | - | - | - / - |
| 7i | `(x \| 4) == 7;` | - | - | - | - | - / - |
| 7j | `if (x \| 0) return 1; return 0;` | - | - | - | - | - / - |
| 7k | `(x & 0) == 0;` | - | - | - | - | - / - |
| 7l | `x & x;` | - | - | - | - | - / - |
| 7m | `x \| x;` | - | - | - | - | - / - |
| 7n | `x ^ x;` | - | - | - | - | - / - |
| 7o | `(u & 0xFF) == 256;` | - | - | tautological-compare | tautological-compare | - / - |
| 7p | `(x \| 4) != 3;` | taut-bitwise-compare | taut-bitwise-compare | tautological-compare | tautological-compare | - / - |
| 7q | `(x \| 1) == 0;` | taut-bitwise-compare | taut-bitwise-compare | tautological-compare | tautological-compare | - / - |
| 7r | `(x & 1) == 2;` | taut-bitwise-compare | taut-bitwise-compare | tautological-compare | tautological-compare | - / - |
| 7s | `(uc & 0x100) == 0;` | - | - | - | - | - / - |
| 7t | `(x & 2) > 1;` | - | - | - | - | - / - |
| 7u | `(u & 0xF0) == 0x0F;` | - | - | tautological-compare | tautological-compare | - / - |
| 7v | `if (!(x \| 4)) return 1; return 0;` | taut-bitwise-compare | taut-bitwise-compare | - | - | - / - |
| 7w | `(4 \| x) == 3;` | taut-bitwise-compare | taut-bitwise-compare | tautological-compare | tautological-compare | - / - |
| 7x | `(2 & x) == 1;` | taut-bitwise-compare | taut-bitwise-compare | tautological-compare | tautological-compare | - / - |
| 7y | `(x ^ 1) == x;` | - | - | - | - | - / - |
| 7z | `(p & 2) == 1;` | taut-bitwise-compare | taut-bitwise-compare | tautological-compare | tautological-compare | - / - |
| b1 | `(x & 0xFF) == 256;` | taut-bitwise-compare | taut-bitwise-compare | tautological-compare | tautological-compare | - / - |
| b4 | `(x \| 0xF0) == 0x0F;` | - | - | tautological-compare | tautological-compare | - / - |
| b5 | `(u & 2) == 1;` | - | - | tautological-compare | tautological-compare | - / - |
| b6 | `(x & 2) == 1u;` | - | - | tautological-compare | tautological-compare | - / - |
| b7 | `(uc & 2) == 1;` | taut-bitwise-compare | taut-bitwise-compare | tautological-compare | - | - / - |
| b10 | `(x \| 3) == 2;` | taut-bitwise-compare | taut-bitwise-compare | tautological-compare | tautological-compare | - / - |
| b11 | `(x \| 3) == 7;` | - | - | - | - | - / - |
| b15 | `(x & 0) == 1;` | taut-bitwise-compare | taut-bitwise-compare | tautological-compare | tautological-compare | - / - |
| b16 | `x \| 4 ? 1 : 2;` | taut-bitwise-compare | taut-bitwise-compare | - | - | - / - |
| b17 | `x \| 4 && a;` | taut-bitwise-compare | taut-bitwise-compare | - | - | - / - |
| b20 | `x \| 4 \|\| a;` | taut-bitwise-compare | taut-bitwise-compare | - | - | - / - |
| b21 | `if ((x \| 4) != 0) return 1; return 0;` | taut-bitwise-compare | taut-bitwise-compare | tautological-compare | tautological-compare | - / - |
| b22 | `if (x \| -1) return 1; return 0;` | taut-bitwise-compare | taut-bitwise-compare | - | - | - / - |
| b23 | `if (4 \| x) return 1; return 0;` | taut-bitwise-compare | taut-bitwise-compare | - | - | - / - |
| b25 | `while (x \| 4) { x--; } return x;` | taut-bitwise-compare | taut-bitwise-compare | - | - | - / - |
| b26 | `!(x \| 4);` | taut-bitwise-compare | taut-bitwise-compare | - | - | - / - |
| b27 | `(x & 1) < 2;` | - | - | - | - | - / - |
| b31 | `(u & 0xFF) == 256;` | - | - | tautological-compare | tautological-compare | - / - |
| b33 | `(x & 1) == -1;` | taut-bitwise-compare | taut-bitwise-compare | tautological-compare | tautological-compare | - / - |
| b34 | `(x \| 1) == -1;` | - | - | - | - | - / - |
| b35 | `(u \| 4) == 3u;` | - | - | tautological-compare | tautological-compare | - / - |
| b40 | `(x & 2) == 1;` | - | - | tautological-compare | tautological-compare | - / - |
| b41 | `(x & 2L) == 1;` | - | - | tautological-compare | - | - / - |
| b43 | `(x & 4) == (2 \| 1);` | - | - | tautological-compare | - | - / - |
| b49 | `if (x & 0) return 1; return 0;` | - | - | - | - | - / - |
| b51 | `x & 0 ? 1 : 2;` | - | - | - | - | - / - |
| 10g | `~uc == 0x0F;` | - | - | sign-compare[E] | sign-compare | - / - |
| 10k | `~uc == 0xF0;` | - | - | sign-compare[E] | sign-compare | - / - |
| 10m | `~us == 0xFFFF;` | - | - | sign-compare[E] | sign-compare | - / - |
| 10G | `~uc == 0xFFFFFF0F;` | sign-compare[E] | sign-compare[E] | sign-compare[E] | sign-compare | - / - |
| 10H | `~uc == -16;` | - | - | - | - | - / - |
| c2 | `~uc > 0;` | - | - | sign-compare[E] | sign-compare | - / - |
| c5 | `~(uc) == 0x0F;` | - | - | sign-compare[E] | sign-compare | - / - |
| c14 | `~uc == vc;` | - | - | sign-compare[E] | sign-compare | - / - |
| c15 | `~uc == (int)vc;` | - | - | sign-compare[E] | sign-compare | - / - |
| c19 | `~uc < -1;` | - | - | - | - | - / - |
| c22 | `~uc == -1;` | - | - | - | - | - / - |
| c26 | `~uc == a + 1;` | - | - | - | - | - / - |
| c30 | `~sc == 0x0F;` | - | - | - | - | - / - |
| c32 | `~(int)uc == 0x0F;` | - | - | sign-compare[E] | sign-compare | - / - |
| c35 | `~ul == 0;` | - | - | - | - | - / - |
| c37 | `~u == 15;` | - | - | - | - | - / - |
| c40 | `~us == 0xFFFF0000;` | sign-compare[E] | sign-compare[E] | sign-compare[E] | sign-compare | - / - |
| c11 | `(unsigned char)~a == 0x0F;` | - | - | - | - | - / - |
| c12 | `~(unsigned char)a == 0x0F;` | - | - | sign-compare[E] | sign-compare | - / - |

(a) **`(x & C1) == C2` / `!=`** with constants and `C2 & ~C1 != 0` (also `C1 = 0`), and **`(x | C1) == C2` / `!=`** with `C1 & ~C2 != 0`, either operand order: GCC (`-Wtautological-compare`) for every operand type (`int`, `unsigned`, `long`, `bool`, `unsigned char`, `x & 2L`, a constant written `0x0F`, and in C `(2 | 1)`), clang (`-Wtautological-bitwise-compare`) only for operands that are `int` after promotion (silent for `unsigned`, `long`, `(x & 2) == 1u`, `b4`-`b6`, `b31`, `b35`, `b40`, `b41`); `<`, `>`, `<=`, `>=` and the non-tautological cases (`7g`-`7i`, `b11`-`b14`, `b32`, `b34`, `(x & 0) == 0`) are silent.
(b) **Clang only:** `x | C`, `C | x` with a non-zero constant `C` used as a truth value (a condition of `if`/`while`/`?:`, an operand of `!`, `&&`, `||`, a `bool` initializer: `7e`, `7v`, `b16`-`b20`, `b22`, `b23`, `b25`, `b26`); silent: `x | 0`, `x | y`, `x & 0` in every context (`7f`, `7j`, `b49`-`b51`).
(c) **GCC only** (`-Wsign-compare`, "comparison of promoted bitwise complement of an unsigned value with constant", `[E]` in C, `-Wall` in C++): a comparison (`== != < > <= >=`) of `~e` with a constant `>= 0` in any spelling (`0x0F`, `15`, `15L`, `'a'`, `2147483647`, a macro) or with a value promoted from an unsigned type narrower
than `int`, when `e` (an explicit cast to `int` or `unsigned char` is looked through: `c12`, `c32`; not `(unsigned char)~a`, `c11`) has an unsigned type of rank below `int` (`unsigned char`, `unsigned short`). **This is the shape the plan listed as silent (`~uc == 0x0F`): it is not, for GCC.** Silent: a negative constant (`~uc == -16`,
`< -1`, `> -256`), a non-constant `int` (`~uc == a + 1`), `~u`, `~ul` (not promoted), `~sc`, `~ch` (signed), `~uc & 0xFF`, `int t = ~uc; t == 15`. `10G`/`c40` (`~uc == 0xFFFFFF0F`: an `unsigned int` constant) is the ordinary `-Wsign-compare` of `int` and `unsigned`, in all four.

**7. `^` spelled as a power** (`-Wxor-used-as-pow`, on by default in all four)

Sweeps (`sweep.py`, `sweep.out`): `2 ^ n` warns in all four exactly for `n` in 0-64 (silent from 65: `2 ^ 255`, `2 ^ 1000`); `10 ^ n` for every `n` probed (0-89, 100, 255); every other left operand (0, 1, 3, 4, 5, 8, 16, 100, 1000, with `n` 2, 3, 8) is silent.

| id | shape | clang C | clang C++ | gcc C | gcc C++ | EDG (C / C++) |
|---|---|---|---|---|---|---|
| 8a | `2 ^ 32;` | xor-used-as-pow[D] | xor-used-as-pow[D] | xor-used-as-pow[D] | xor-used-as-pow[D] | - / - |
| 8b | `2 ^ 8;` | xor-used-as-pow[D] | xor-used-as-pow[D] | xor-used-as-pow[D] | xor-used-as-pow[D] | - / - |
| 8c | `10 ^ 3;` | xor-used-as-pow[D] | xor-used-as-pow[D] | xor-used-as-pow[D] | xor-used-as-pow[D] | - / - |
| 8d | `2 ^ x;` | - | - | - | - | - / - |
| 8e | `0x2 ^ 32;` | - | - | - | - | - / - |
| 8f | `2u ^ 32;` | - | - | xor-used-as-pow[D] | xor-used-as-pow[D] | - / - |
| 8g | `2 ^ 0x20;` | - | - | - | - | - / - |
| 8h | `1 ^ 32;` | - | - | - | - | - / - |
| 8i | `POWER(8);` | - | - | - | - | - / - |
| 8s | `2 ^ N32;` | xor-used-as-pow[D] | xor-used-as-pow[D] | - | - | - / - |
| 8t | `0x10 ^ 2;` | - | - | - | - | - / - |
| 8z | `3 ^ 5;` | - | - | - | - | - / - |
| 8F | `2 ^ 255;` | - | - | - | - | - / - |
| 8H | `4 ^ 2;` | - | - | - | - | - / - |
| 8v | `2L ^ 32;` | - | - | xor-used-as-pow[D] | xor-used-as-pow[D] | - / - |
| 8w | `2 ^ 32u;` | - | - | xor-used-as-pow[D] | xor-used-as-pow[D] | - / - |
| x_sp1 | `2 ^ 8;` | xor-used-as-pow[D] | xor-used-as-pow[D] | xor-used-as-pow[D] | xor-used-as-pow[D] | - / - |
| x_sp2 | `0x2 ^ 8;` | - | - | - | - | - / - |
| x_sp3 | `2 ^ 0x8;` | - | - | - | - | - / - |
| x_sp4 | `02 ^ 8;` | - | - | - | - | - / - |
| x_sp5 | `2 ^ 010;` | - | - | - | - | - / - |
| x_sp6 | `0b10 ^ 8;` | - | - | - | - | - / - |
| x_sp7 | `2 ^ 0b1000;` | - | - | - | - | - / - |
| x_sp8 | `2u ^ 8;` | - | - | xor-used-as-pow[D] | xor-used-as-pow[D] | - / - |
| x_sp9 | `2 ^ 8u;` | - | - | xor-used-as-pow[D] | xor-used-as-pow[D] | - / - |
| x_sp12 | `(2) ^ 8;` | - | - | xor-used-as-pow[D] | - | - / - |
| x_sp13 | `2 ^ (8);` | - | - | xor-used-as-pow[D] | - | - / - |
| x_sp14 | `2 ^ +8;` | xor-used-as-pow[D] | xor-used-as-pow[D] | - | - | - / - |
| x_sp15 | `2 ^ (int)8;` | - | - | - | - | - / - |
| x_sp16 | `TWO ^ 8;` | xor-used-as-pow[D] | xor-used-as-pow[D] | - | - | - / - |
| x_sp17 | `2 ^ N32;` | xor-used-as-pow[D] | xor-used-as-pow[D] | - | - | - / - |
| x_sp18 | `2 ^ E32;` | - | - | - | - | - / - |
| x_sp19 | `TWO ^ N32;` | - | - | - | - | - / - |
| x_sp20 | `2 ^ '\x08';` | - | - | - | - | - / - |
| x_sp21 | `2 ^ (1 + 7);` | - | - | - | - | - / - |
| x_sp22 | `(1 + 1) ^ 8;` | - | - | - | - | - / - |
| x_sp24 | `10 ^ 0x3;` | - | - | - | - | - / - |
| x_sp26 | `10 ^ 3u;` | - | - | xor-used-as-pow[D] | xor-used-as-pow[D] | - / - |
| x_sp28 | `2 ^ -1;` | - | - | - | - | - / - |
| x_sp29 | `-2 ^ 8;` | - | - | - | - | - / - |
| x_sp30 | `2 ^ 8 ^ 1;` | xor-used-as-pow[D] | xor-used-as-pow[D] | xor-used-as-pow[D] | xor-used-as-pow[D] | - / - |
| x_sp31 | `1 ^ 2 ^ 8;` | - | - | - | - | - / - |
| x_sp32 | `x ^ 2 ^ 8;` | - | - | - | - | - / - |
| x_sp37 | `(unsigned char)2 ^ 8;` | - | - | - | - | - / - |
| x_sp40 | `'\x02' ^ 8;` | - | - | - | - | - / - |
| x_sp41 | `2 ^ 8 * 1;` | - | - | - | - | - / - |
| x_mac1 | `POWER(8);` | - | - | - | - | - / - |
| x_mac2 | `POWER(N32);` | - | - | - | - | - / - |
| k5 | `static_assert(2 ^ 8, "");` | xor-used-as-pow[D] | xor-used-as-pow[D] | xor-used-as-pow[D] | xor-used-as-pow[D] | - / - |

Spelling decides the compilers' verdict: **clang** needs two unsuffixed decimal literal tokens, the left not parenthesized, the `^` written outside any macro and at least one operand written in the source (`TWO ^ 8` and `2 ^ N32` warn, `TWO ^ N32` and the macro body `POWER(8)` do not; `2 ^ +8` warns; `sp14`, `sp16`, `sp17`, `sp19`); **GCC** also accepts a suffix on either (`2u`, `8L`, `sp8`-`sp11`, `sp26`, `sp27`), a parenthesized literal in C (`(2) ^ 8`, `2 ^ (8)`), but not a literal that comes from a macro; hex, octal, binary,
character, `enum` and `const` operands, `(int)8`, `(1 + 1) ^ 8`, `2 ^ -1`, `-2 ^ 8` are silent everywhere, and so are `POWER(8)` (a macro body `2 ^ n`). `2 ^ 8 ^ 1` warns on the inner `2 ^ 8` (`sp30`); `1 ^ 2 ^ 8` and `x ^ 2 ^ 8` do not. The IL has constants, not tokens (EDG folds `2 ^ 8`; the expression is kept, P38),
so the rule below rejects the broader shape.

**8. A compound bitwise assignment as the whole condition** (clang `-Wparentheses`, on by default; GCC never)

| id | shape | clang C | clang C++ | gcc C | gcc C++ | EDG (C / C++) |
|---|---|---|---|---|---|---|
| 9a | `if (x \|= a) return x; return 0;` | parentheses[D] | parentheses[D] | - | - | - / - |
| 9b | `while (x &= a) { a--; } return x;` | - | - | - | - | - / - |
| 9c | `if ((x \|= a)) return x; return 0;` | - | - | - | - | - / - |
| 9d | `int y; y = x \|= a; return y;` | - | - | - | - | - / - |
| 9e | `if (x = a & b) return x; return 0;` | parentheses[D] | parentheses[D] | parentheses | parentheses | - / - |
| 9f | `if (x ^= a) return x; return 0;` | - | - | - | - | - / - |
| 9g | `if (x &= a) return x; return 0;` | - | - | - | - | - / - |
| 9h | `x \|= a;` | - | - | - | - | - / - |
| 9i | `for (; x &= a;) { a--; } return x;` | - | - | - | - | - / - |
| 9j | `do { a--; } while (x \|= a); return x;` | parentheses[D] | parentheses[D] | - | - | - / - |
| 9l | `(x \|= a) ? 1 : 2;` | - | - | - | - | - / - |
| 9m | `if (x = a \| b) return x; return 0;` | parentheses[D] | parentheses[D] | parentheses | parentheses | - / - |
| 9n | `x \|= a; return x;` | - | - | - | - | - / - |
| 9o | `(x \|= a) + 1; return x;` | unused-value[D] | unused-value[D] | unused-value | unused-value | - / - |
| 9p | `int y = x \|= a; return y;` | - | - | - | - | - / - |
| 9q | `if ((x = a & b)) return x; return 0;` | - | - | - | - | - / - |
| 9r | `x = a & b;` | - | - | - | - | - / - |
| 9s | `if (x += a) return x; return 0;` | - | - | - | - | - / - |
| 9t | `!(x \|= a);` | - | - | - | - | - / - |
| 9u | `(x \|= a) && a;` | - | - | - | - | - / - |
| 9v | `while (x \|= a) { a--; } return x;` | parentheses[D] | parentheses[D] | - | - | - / - |
| 9w | `if (x \|= a) return 1; return 0;` | parentheses[D] | parentheses[D] | - | - | - / - |
| 9x | `if (x <<= a) return x; return 0;` | - | - | - | - | - / - |
| 9y | `if (x >>= a) return x; return 0;` | - | - | - | - | - / - |
| 9z | `if (p &= q) return 1; return 0;` | - | - | - | - | - / - |
| o1 | `if (x \|= a) return x; return 0;` | parentheses[D] | parentheses[D] | - | - | - / - |
| o2 | `if ((x \|= a)) return x; return 0;` | - | - | - | - | - / - |
| o3 | `if ((x \|= a, x)) return x; return 0;` | - | - | - | - | - / - |
| o4 | `if ((x \|= a) && b) return x; return 0;` | - | - | - | - | - / - |
| o5 | `if (x \|= 1) return x; return 0;` | parentheses[D] | parentheses[D] | - | - | - / - |
| o6 | `if (!(x \|= a)) return x; return 0;` | - | - | - | - | - / - |
| o7 | `if (x \|= a) return 1; return 0;` | parentheses[D] | parentheses[D] | - | - | - / - |
| o8 | `if (p \|= q) return 1; return 0;` | parentheses[D] | parentheses[D] | - | - | - / - |
| o9 | `if (a && (x \|= b)) return x; return 0;` | - | - | - | - | - / - |
| o10 | `if (x \|= a && b) return x; return 0;` | parentheses[D] | parentheses[D] | - | - | - / - |
| o13 | `for (; x \|= a;) { a--; } return x;` | parentheses[D] | parentheses[D] | - | - | - / - |
| o14 | `do { a--; } while (x \|= a); return x;` | parentheses[D] | parentheses[D] | - | - | - / - |
| o15 | `do { a--; } while ((x \|= a)); return x;` | - | - | - | - | - / - |
| o16 | `x \|= a ? 1 : 2;` | - | - | - | - | - / - |
| o17 | `if (x ^= a) return x; return 0;` | - | - | - | - | - / - |
| o18 | `if (x &= a) return x; return 0;` | - | - | - | - | - / - |
| o19 | `if (x <<= a) return x; return 0;` | - | - | - | - | - / - |
| o20 | `if (x -= a) return x; return 0;` | - | - | - | - | - / - |
| o21 | `if (x *= a) return x; return 0;` | - | - | - | - | - / - |
| o22 | `if (x %= a) return x; return 0;` | - | - | - | - | - / - |
| o23 | `if (x /= a) return x; return 0;` | - | - | - | - | - / - |
| o24 | `if (x \|= a) {} return x;` | parentheses[D] | parentheses[D] | - | - | - / - |
| o25 | `if (x \|= a != 0) return x; return 0;` | parentheses[D] | parentheses[D] | - | - | - / - |
| o26 | `int y = (x \|= a) ? 1 : 2; return y;` | - | - | - | - | - / - |
| o27 | `int y = x \|= a ? 1 : 2; return y;` | - | - | - | - | - / - |
| o28 | `if (x \|= a) return 1; else return 0;` | parentheses[D] | parentheses[D] | - | - | - / - |
| o29 | `switch (x \|= a) { case 0: return 1; default: return 0; }` | - | - | - | - | - / - |
| o30 | `if ((x \|= a) != 0) return x; return 0;` | - | - | - | - | - / - |

Clang warns on `|=` as the entire condition of `if`, `while`, `for` or `do` without extra parentheses (also on `bool` `p |= q`, `x |= a != 0`); **never** on `&=`, `^=`, `<<=`, `>>=`, `-=`, `*=`, `/=`, `%=`, nor under `switch`, `!`, `&&`, `?:`, a comma, or `(x |= a) != 0`. GCC is silent. The plain `x = a & b` condition warns in all four, as 0020 item 5 says (`9e`, `9m`);
`(x |= a) + 1;` is the existing discarded-value rule (`9o`).

**9. Self-assignment** (clang `-Wself-assign`, in `-Wall`; GCC never; 0021 item 8)

| id | shape | clang C | clang C++ | gcc C | gcc C++ | EDG (C / C++) |
|---|---|---|---|---|---|---|
| s_set | `x = x; return x;` | self-assign | self-assign | - | - | - / - |
| s_and | `x &= x; return x;` | self-assign | self-assign | - | - | - / - |
| s_or | `x \|= x; return x;` | self-assign | self-assign | - | - | - / - |
| s_xor | `x ^= x; return x;` | - | - | - | - | - / - |
| s_add | `x += x; return x;` | - | - | - | - | - / - |
| s_sub | `x -= x; return x;` | - | - | - | - | - / - |
| s_mul | `x *= x; return x;` | - | - | - | - | - / - |
| s_div | `x /= x; return x;` | - | - | - | - | - / - |
| s_and_paren | `x &= (x); return x;` | self-assign | self-assign | - | - | - / - |
| s_or_paren | `x \|= (x); return x;` | self-assign | self-assign | - | - | - / - |
| s_and_uc | `uc &= uc; return uc;` | self-assign | self-assign | - | - | - / - |
| s_or_sc | `sc \|= sc; return sc;` | self-assign | self-assign | - | - | - / - |
| s_and_cast | `x &= (int)x; return x;` | - | - | - | - | - / - |
| s_or_cast | `x \|= (int)x; return x;` | - | - | - | - | - / - |
| s_and_u | `u &= u; return (int)u;` | self-assign | self-assign | - | - | - / - |
| s_and_bool | `p &= p; return p;` | self-assign | self-assign | - | - | - / - |
| s_or_bool | `p \|= p; return p;` | self-assign | self-assign | - | - | - / - |
| s_xor_bool | `p ^= p; return p;` | - | - | - | - | - / - |
| s_and_other | `x &= y; return x;` | - | - | - | - | - / - |
| s_and_plus | `x &= x + 0; return x;` | - | - | - | - | - / - |
| s_and_stmt_expr_value | `int y; y = x &= x; return y;` | self-assign | self-assign | - | - | - / - |
| s_and_cond | `if ((x &= x)) return 1; return 0;` | self-assign | self-assign | - | - | - / - |

Clang's `-Wself-assign` covers `x &= x` and `x |= x` exactly as `x = x` (through parentheses, promotions, `unsigned`, `bool`; an explicit cast hides it); `^=`, `+=`, `-=`, `*=`, `/=`, `%=`, `<<=`, `>>=` are silent.

**10. Shapes that stay silent in all four** (`-` everywhere, except where noted)

| id | shape | clang C | clang C++ | gcc C | gcc C++ | EDG (C / C++) |
|---|---|---|---|---|---|---|
| 10a | `x \|= y == z; return x;` | - | - | - | - | - / - |
| 10c | `p ^ q;` | - | - | - | - | - / - |
| 10d | `!a & !b;` | - | - | - | - | - / - |
| 10e | `(a & b) == c;` | - | - | - | - | - / - |
| 10f | `a \| (b & c);` | - | - | - | - | - / - |
| 10h | `(int)(u & ~7);` | - | - | - | - | - / - |
| 10i | `sc \|= 0x80; return sc;` | - | - | - | - | - / - |
| 10j | `uc \|= 0x80; return uc;` | - | - | - | - | - / - |
| 10l | `~uc & 0xFF;` | - | - | - | - | - / - |
| 10n | `(unsigned char)~uc;` | - | - | - | - | - / - |
| 10o | `~sc == 0x0F;` | - | - | - | - | - / - |
| 10p | `x & ~u;` | - | - | - | - | - / - |
| 10q | `x \|= 1u << 3; return x;` | - | - | - | - | - / - |
| 10r | `u \|= 0x80000000; return (int)u;` | - | - | - | - | - / - |
| 10s | `x & (int)(1u << 31);` | - | - | - | - | - / - |
| 10t | `(int)(u & ~0u);` | - | - | - | - | - / - |
| 10u | `x & -1;` | - | - | - | - | - / - |
| 10v | `x \| -1;` | - | - | - | - | - / - |
| 10w | `(int)(u & -1);` | - | - | - | - | #68 / #68 |
| 10x | `x &= 0xFF; return x;` | - | - | - | - | - / - |
| 10y | `x &= ~0xFF; return x;` | - | - | - | - | - / - |
| 10z | `uc &= ~1; return uc;` | - | - | - | - | - / - |
| 10A | `uc = ~uc; return uc;` | - | - | - | - | - / - |
| 10F | `(~uc & 0xFF) == 0x0F;` | - | - | - | - | - / - |
| 10M | `x ^= y; return x;` | - | - | - | - | - / - |
| 10N | `x &= y; return x;` | - | - | - | - | - / - |
| 10O | `x \|= y; return x;` | - | - | - | - | - / - |
| 10Q | `x ^= x; return x;` | - | - | - | - | - / - |
| 3d | `!x & !y;` | - | - | - | - | - / - |
| 3e | `!x & (y > 0);` | - | - | - | - | - / - |
| 3f | `x & !y;` | - | - | - | - | - / - |
| 3h | `!(x & y);` | - | - | - | - | - / - |
| 3m | `(!x) & y;` | - | - | - | - | - / - |
| 3n | `y \| !x;` | - | - | - | - | - / - |
| 6g | `a & b;` | - | - | - | - | - / - |
| 6h | `(a > 0) & (b > 0);` | - | - | - | - | - / - |
| 7l | `x & x;` | - | - | - | - | - / - |
| 7m | `x \| x;` | - | - | - | - | - / - |
| 7n | `x ^ x;` | - | - | - | - | - / - |

`x |= y == z` (`10a`), `b1 ^ b2`, `!a & !b`, `(a & b) == c`, `a | (b & c)`, `u & ~7`, `sc |= 0x80` (a narrowing assignment: `-Wconversion` is not in `-Wall`), `u |= 0x80000000`, `x & -1`, `x | -1`, `x & ~u`, `x |= 1u << 3`, `~uc & 0xFF`, `x ^= x`: all silent. The two exceptions are `x &= x` and `x |= x`
(clang `-Wself-assign`, item 9), and `~uc == 0x0F`/`~uc == 0xF0`/`~us == 0xFFFF` (`10g`, `10k`, `10m`: GCC `-Wsign-compare`, item 6c).

**11. Measure only (not bitwise; for the owner's later question)**

| id | shape | clang C | clang C++ | gcc C | gcc C++ | EDG (C / C++) |
|---|---|---|---|---|---|---|
| 11a | `a && b \|\| c;` | logical-op-parentheses | logical-op-parentheses | parentheses | parentheses | - / - |
| 11b | `a \|\| b && c;` | logical-op-parentheses | logical-op-parentheses | parentheses | parentheses | - / - |
| 11c | `a + b << c;` | shift-op-parentheses[D] | shift-op-parentheses[D] | parentheses | parentheses | - / - |
| 11d | `a << b + c;` | shift-op-parentheses[D] | shift-op-parentheses[D] | parentheses | parentheses | - / - |
| 11m | `a - b << c;` | shift-op-parentheses[D] | shift-op-parentheses[D] | parentheses | parentheses | - / - |
| 11n | `a << b - c;` | shift-op-parentheses[D] | shift-op-parentheses[D] | parentheses | parentheses | - / - |
| 11o | `a + b >> c;` | shift-op-parentheses[D] | shift-op-parentheses[D] | parentheses | parentheses | - / - |
| 11p | `a >> b + c;` | shift-op-parentheses[D] | shift-op-parentheses[D] | parentheses | parentheses | - / - |
| 11e | `!x == y;` | logical-not-parentheses[D] | logical-not-parentheses[D] | logical-not-parentheses | logical-not-parentheses | - / - |
| 11t | `!a == b;` | logical-not-parentheses[D] | logical-not-parentheses[D] | logical-not-parentheses | logical-not-parentheses | - / - |
| 11f | `x == y == z;` | - | - | parentheses | parentheses | - / - |
| 11j | `x < y < z;` | parentheses[D] | parentheses[D] | parentheses | parentheses | - / - |
| 11z | `a < b == 1;` | - | - | parentheses | parentheses | - / - |
| 11g | `x == x;` | tautological-compare[D] | tautological-compare[D] | tautological-compare | tautological-compare | - / - |
| 11k | `x != x;` | tautological-compare[D] | tautological-compare[D] | tautological-compare | tautological-compare | - / - |
| 11v | `x > x;` | tautological-compare[D] | tautological-compare[D] | tautological-compare | tautological-compare | - / - |
| 11h | `u >= 0;` | - | - | type-limits[E] | type-limits[E] | #186 / #186 |
| 11l | `u < 0;` | - | - | type-limits[E] | type-limits[E] | #186 / #186 |
| 11i | `if (a << b) return 1; return 0;` | int-in-bool-context | int-in-bool-context | int-in-bool-context | int-in-bool-context | - / - |
| 11y | `a << b ? 1 : 2;` | int-in-bool-context | int-in-bool-context | int-in-bool-context | int-in-bool-context | - / - |
| 11x | `if (a * b) return 1; return 0;` | - | - | int-in-bool-context | int-in-bool-context | - / - |
| 11q | `(a && b) \|\| c;` | - | - | - | - | - / - |
| 11r | `a && (b \|\| c);` | - | - | - | - | - / - |
| 11s | `a && b && c;` | - | - | - | - | - / - |
| 11u | `!(a == b);` | - | - | - | - | - / - |
| 11w | `x - x;` | - | - | - | - | - / - |

All four warn: `a && b || c`, `a || b && c` (`-Wlogical-op-parentheses`/`-Wparentheses`, `-Wall`), `a + b << c`, `a << b + c` and the `-`/`>>` variants (`-Wshift-op-parentheses` on by default in clang; `-Wparentheses` in GCC), `!x == y` (`-Wlogical-not-parentheses`), `x == x`, `x != x`, `x > x`
(`-Wtautological-compare`), `if (a << b)` and `a << b ? 1 : 2` (`-Wint-in-bool-context`). **Only GCC:** `x == y == z` and `a < b == 1` (`-Wparentheses`), `if (a * b)` (`-Wint-in-bool-context`), `u >= 0` and `u < 0` (`-Wtype-limits`, `[E]`; EDG's #186 does warn, in both modes).
**`x < y < z` is an error in clang** (C and C++, by default, `-Wparentheses`: "chained comparison 'X < Y < Z' does not behave the same as a mathematical expression"; GCC only warns). Silent: `(a && b) || c`, `a && (b || c)`, `a && b && c`, `!(a == b)`, `x - x`, `x == y == z` in clang.

#### The rules (decision 0023's list; each states what the IL must show)

The owner's rule (0020 item 5, 0022 item 5): the union of what clang and GCC warn about, in both modes; where the IL cannot see what the compilers see (a literal's spelling, macro use) cerastes rejects the broader shape. All rules apply in code **and** in contracts (the compilers never see the latter, so this is cerastes's stricter reading).
Operand "kinds" are after peeling parentheses (`is_parenthesized`) and `compiler_generated` casts only. Seven new rules, two extensions of existing ones.

1. **`bitwise_operand_parentheses`.** A binary `eok_and`, `eok_or` or `eok_xor` has an operand (left or right) that is not `is_parenthesized` and is (a) a comparison operation (`eok_eq/ne/lt/gt/le/ge`), or (b) binary `eok_add`/`eok_sub`, or (c) a binary `eok_and/or/xor` whose operator differs from the parent's
   (`&` under `|` or `^`; `^` under `|`; same-operator chains are fine). Shifts, `*`, `/`, `%`, unary operators and calls are not matched. Spelling: none; macros: the flag is the only thing read (clang warns inside macros, GCC mostly not, `+`/`-` in a macro neither: one rule for all).
   (a) and (c) are warned by both compilers, (b) by GCC only; `a == b & c == d` is rejected although clang is silent (GCC warns). `(a < b) | (c < d)` (parenthesized) is accepted.
2. **`logical_not_operand`.** A binary `eok_and` or `eok_or` whose **left** operand is an unparenthesized `eok_lnot` (any operand, itself a truth value or not). Accepted: the right operand is a truth value T (a comparison, `!`, `&&`, `||`, a bool-typed expression or `true`/`false`, with no explicit cast to a non-bool type in between);
   for `|` also a right operand that is the literal `0` or `1` (any suffix) or an explicit cast of a comparison (GCC is silent, clang never warns on `|`). Everything else is rejected, including `!x & 1` and `!x & (int)(y > 0)` (clang warns), `!!x & y`, `!(x > 0) & y`, `!p & y` (GCC warns). `^` is never matched, nor a `!` on the right.
3. **`bool_complement`.** An `eok_not` (bitwise `~`) whose operand, after peeling parentheses, compiler-generated casts and a unary plus, is a truth value T extended by `&`/`|`/`^` of two truth values (clang) and a `?:` with bool arms; a constant folded from such an expression counts (`~true`, `~(1 == 1)`). An explicit `(int)`/`(unsigned)` cast is a barrier,
   `(bool)` is not. `~0`, `~1`, `~x`, `~(a & 1)`, `~(a ? 1 : 0)` are accepted. (Both compilers for the core, clang or g++ for the rest.)
4. **`bitwise_condition_precedence`.** A `?:` whose condition is not `is_parenthesized` and is a binary `eok_and` or `eok_or` (not `^`) whose **right** operand is a truth value T (explicit casts are a barrier; `~b` and `?:` are not T). Clang only; `a & b ? c : d` with ordinary operands is accepted (all four silent).
5. **`bitwise_instead_of_logical`.** A binary `eok_and` or `eok_or` (not `^`, not the compound forms) whose operands are both truth values T and **both** contain a side effect (a call, `++`/`--`, an assignment; only calls and `++` probed). Clang only; one side effect, `int` operands and explicit casts to `int` are accepted.
6. **`bitwise_constant_comparison`**, three clauses. (a) `eok_eq`/`eok_ne` between a constant `C2` and a binary `x & C1` (resp. `x | C1`), `C1` constant, with `C2 & ~C1 != 0` (resp. `C1 & ~C2 != 0`), either order, any integer type (GCC; clang only for `int`-after-promotion), the values EDG's folded ones (spelling, macros, `(2 | 1)` irrelevant).
   (b) `x | C` or `C | x`, `C` a non-zero constant, in a truth context (a condition, an operand of `!`, `&&`, `||`, a conversion to `bool`; clang). (c) Any of the six comparisons with an operand `~e` where, looking through explicit casts to an integer type, `e` has an unsigned type of rank below `int`, and the other operand is a constant `>= 0` or has such a promoted unsigned type (GCC `-Wsign-compare`);
   a negative constant or a non-constant signed operand is accepted. Clause (c) rejects `~uc == 0x0F`, which the plan expected to stay silent.
7. **`xor_as_power`.** A binary `eok_xor` whose operands are both integer constants (any spelling, suffix, parenthesization, macro, `enum` or `const` origin: **broader** than all four compilers) with the left operand's value 2 and the right's in 0..64, or the left 10 and the right >= 0 (probed to 255). Silent in all four, rejected by cerastes:
   `0x2 ^ 8`, `02 ^ 8`, `2 ^ 0b1000`, `2 ^ E32`, `(1 + 1) ^ 8`, `2 ^ (int)8`, and the macro-body form `POWER(8)` once expanded. Accepted: any other left value, a right operand above 64 for `2`, a non-constant operand (`2 ^ x`, `x ^ 2 ^ 8` unless the folded part is a constant `2 ^ 8`).
- **E1. `assignment_as_condition` (0020 item 5), extended:** also `|=` (unparenthesized, the whole condition of `if`/`while`/`do`/`for`, any operand type, `bool` included; clang). Not `&=`, `^=` or any other compound, not `switch`.
- **E2. `self_assignment` (0021 item 8), extended:** also `x &= x` and `x |= x` (same variable after the generated conversions, no macro involvement, an explicit cast hides it, parentheses do not); not `^=`, `+=`, `-=`, `*=`, `/=`, `%=`, `<<=`, `>>=`.

Rules 2, 3 and 5 share the notion T and the peeling, which the encoder implements once. `-Wunused-value` shapes of the bitwise operators (a discarded `a & b;`, `x |= 1;`) follow 0020 item 5's existing rules (`9o`: `(x |= a) + 1;` already rejected by `value_not_used`); `a & b;` as a statement has no side effect and stays accepted like `x + 1;`.

#### Q33: the header sweep for `--diag_error=1422,1654,26,3255`

No header fails. The directly included headers of `tests/scalar`, `tests/annotations` and the doc examples are `<cerastes.h>`, `<stdlib.h>`, `<assert.h>`, `<stdio.h>`, `<stdbool.h>`, `<limits.h>`, `<sys/wait.h>`, `<cstdlib>`, `<cassert>`, `<cstdio>`, `<exception>`, `<coroutine>` and the helper headers
under `tests/scalar/headers`. `q33/sweep.py` parses each C header in C23 and C++20, each C++ header in C++20, with and without the four numbers (both runs with `--display_error_number`): 19 runs, exit 0 and no diagnostic in each (`q33/sweep.out`). Two further facts:
(1) EDG suppresses diagnostics in system headers **even when `--diag_error` raises them**: a `--sys_include` header with `'ab'` is silent (exit 0), the same header through `-I` is error #1422 (`q33/usesys.c`). The four numbers therefore bind only the primary file and user headers (`-I`, quotes), which the sweep also covers:
(2) no verify test, doc example, helper header or `include/cerastes.h` contains a character literal at all (a grep for `'x'`, `'ab'`, `'\x...'` outside comments finds none), so no test trips #1422, #1654, #26 or #3255. Probe: `'ab'` is #1422 (warning by default) in both modes, `'abcde'` #1654, `L'ab'` #26 in C (error already in C++), `'\x1234'` #27 (an error by default).

### S21: a closed form for `& | ^` on mathematical integers (M4.4b1): **PASS for 32, 64 and 128 bits with the hierarchical form `Hn8`; the proposed flat forms (A1, A1h, A2, A2h) FAIL on symbolic operands**

All files, generators and logs: `build/m4.4b1/s21/` (`gen.py`, `gen_split.py`, `gen_h.py` produce the Viper; `run.py` runs it; `claims.py`, `leaf_lemmas.py`, `t_pre.py` are the side experiments; `vpr/` has every file run, `out/` the last Silicon output of each, `results.tsv`, `logs/`). Silicon is run as `src/driver/silicon.cpp:106` runs it: `java -Xss128m -jar silicon.jar --z3Exe z3 --numberOfErrorsToReport 0 program.vpr`, clean environment, pinned JDK 17, Silicon `6b540aa4`, Z3 4.16.0. Timing floor on this machine, a file with one trivial method: **1.3 s** (3 runs, `trivial.vpr`). A baseline run was loading the machine for the first hour (load average up to 174); every number below is from runs made after it ended, at load below 15, and the timing table (3 repetitions) is from a quiet machine.

**1. Semantics.** `semantics.vpr` (12 asserts, Silicon `finished verification successfully`, 1.49 s): Viper's `\` is floor division and `%` the non-negative remainder for a positive divisor, including negative dividends: `(-7) \ 2 == -4`, `(-7) % 2 == 1`, `(-1) \ 2 == -1`, `(-9223372036854775808) \ 4294967296 == -2147483648`, `(-4294967297) % 4294967296 == 4294967295`. Its twin `semantics_fail.vpr` (`(-7) \ 2 == -3`) gives 1 error (`Assert might fail … program.vpr@3.10--3.26`).

**2. The proposed formula is correct** (`formula.py`): `c_and_N(a,b) = Σ_{k<N} 2^k·[bit_k(a) ∧ bit_k(b)] − (a<0 ∧ b<0 ? 2^N : 0)`, `or = a + b − and`, `xor = a + b − 2·and`, with `bit_k(a) = (a \ 2^k) % 2`: 0 mismatches against Python's `& | ^` on 27033 operand pairs for N = 32, 64, 128 (the type's MIN/MAX, −1, 2^N−1, −2^N, random in [−2^N, 2^N), random signed, random unsigned). Hand cases: `(−1) ^ INT_MAX == INT_MIN`, `INT_MIN & −1 == INT_MIN`, `(−1) & 12345 == 12345`, `(−8) & 13 == 8`: all True. The same formula, as Viper, decides every closed constant item below (A1, 32/64/128 bits: 2.0–2.8 s). **It is not usable for symbolic operands**, see 3.

**3. Variants A1, A1h, A2, A2h (the orchestrator's list), N = 32** (one repetition, timeout 40 s; "T/O" is a timeout, i.e. no verdict):

| item | A1 | A1h | A2 (let chain, flat) | A2h |
|---|---|---|---|---|
| closed constants, prologue asserts, neg/MIN (6 files) | 1.8–1.9 s, as expected | not run | not run | not run |
| `x >= 0 ==> and(x, 1) == x % 2` (+ twin) | 1.95 s | | | |
| `xor(x, x) == 0` | **T/O** | **T/O (30 s)** | **T/O (30 s)** | **T/O (30 s)** |
| signed range of `and/or/xor` of two signed | **T/O** | **T/O** | **T/O** | **T/O** |
| unsigned range, `ten_ops_*`, `xor_invol`, `low_bit` | T/O | | | |

Scaling of A1 and A2 on `xor(x,x)==0` and the signed range property (60 s timeout, bits N): A1 xor_self 8: 1.8 s, 10: 6.9 s, 12: T/O; A1 range 8: 9.7 s, 10: T/O; A2 xor_self 8..12: 1.7–4 s, 16: 5.4 s, 20: T/O; A2 range 8: 1.8 s, 12: 4.2 s, 16: T/O. So both are exponential in N for symbolic operands (Z3 case-splits the per-bit `ite`s and the `%`/`\` atoms); 64 bits cannot be completed with them, and a bounded recursion or SMT bit-vectors was not needed (below). Per-function cost of the flat forms is also quadratic: a prelude of one 32/64/128-bit function costs 0.6/1.7/6.5 s over the floor with Silicon doing nothing but reading it (`t_pre.py`).

**4. What works: `Hn8`.** Three ideas, each measured:
1. **Bits from a floor-division chain** `q0 = a`, `q_{k+1} = q_k \ 2`, `bit_k = q_k − 2·q_{k+1}` (no `%`, no `2^k` literals; the chain on a negative `a` is the infinite two's-complement extension), and **no `(a<0 && b<0)` correction**: the signed function gives the top bit the weight `−2^(N−1)`, so its result is in the signed range by construction; the unsigned function gives it `+2^(N−1)`. Hence **two sets of functions** (`c_and_N` for signed computed types, `c_uand_N` for unsigned ones), as `c_div`/`c_udiv` are split today. Both are total and read the low N bits of two's complement operands (exact on [−2^(N−1), 2^(N−1)) resp. [0, 2^N)). `hsim.py` checks the whole construction (every width, leaf widths 4/8/16): 0 mismatches.
2. **Hierarchy.** Only an 8-bit leaf has a bit chain. An N-bit function is `c_u<op>_{N/2}(a, b) + 2^(N/2) * c_<u?><op>_{N/2}(a \ 2^(N/2), b \ 2^(N/2))`: a low half (the low bits of `a`, unsigned) plus a high half of the same signedness as the parent. No sizeable function body exists, so the prelude cost is linear (all six functions of one width: 2.0, 2.1, 2.2 s for 32, 64, 128 against 4.0, 10.3, 45 s flat).
3. **Per bit `ite`**: and: `((ba + bb == 2) ? 1 : 0)`, or: `>= 1`, xor: `== 1`, each op its own function (not derived from `and`). No postcondition on any function (decision 0012 item 11 stays as it is): Z3 derives the range of a result when the property needs it (`range_s`).

**Timing, `Hn8`, whole battery, three repetitions each (median/max seconds; "T/O" = no verdict in 20 s, ×3 repetitions stop at the first):**

| item | N=32 | N=64 | N=128 |
|---|---|---|---|
| consts (12 exact constants incl. `xor_64(-1, 2^63-1) == -2^63`, MIN & -1) | 2.03/2.28 | 2.09/2.19 | 2.20/2.26 |
| consts_fail (`or(240,15) == 256`: exactly 1 error) | 2.02/2.16 | 2.07/2.21 | 2.14/2.26 |
| prologue (K=0xF0\|0x0F, `~0`, enumerator A\|B, case value 0x80\|1, nested calls) | 2.13/2.19 | 2.40/2.50 | 4.56/4.74 |
| prologue_fail (`or(240,256) <= 255`: exactly 1 error) | 2.03/2.14 | 2.05/2.17 | 2.17/2.26 |
| neg_min / neg_min_fail (negative operands, results equal to MIN) | 2.01/2.13, 1.99/2.11 | 2.07/2.15, 2.05/2.14 | 2.11/2.29, 2.09/2.21 |
| `x>=0 ==> and(x,1) == x%2` / twin (`== 1`) | 1.65/1.75, 1.67/1.77 | 1.66/1.77, 1.67/1.81 | 1.68/1.79, 1.73/1.84 |
| `xor(x,x) == 0` / twin | 1.66/1.76, 1.65/1.76 | 1.66/1.75, 1.66/1.77 | 1.69/1.83, 1.68/1.84 |
| signed range of and/or/xor / twin (`0 <= and(x,y)`) | 2.02/2.12, 2.14/2.27 | 2.08/2.18, 2.13/2.22 | 2.22/2.29, 2.50/2.62 |
| 10 symbolic ops, result in range / twin (`t10 == t9 + 1`) | 2.01/2.15, 2.23/2.31 | 2.12/2.23, 3.09/3.21 | 2.29/2.37, **T/O** |
| false property `or(x,y) >= and(x,y)` (must fail) | 1.85/1.95 | 1.95/2.05 | 2.14/2.23 |
| `and(-1,x)==x && or(x,0)==x && and(x,0)==0` twin (`== x + 1`) | 1.66/1.75 | 1.67/1.77 | 1.72/1.84 |

Every must-pass item passes at every width, every twin and the false property fail as expected (one error where one is predicted) except the 128-bit `ten_ops_fail`. The median budget (< 2 s) is met by the symbolic items and misses by 0.0–0.3 s for the constant items, whose floor here is the 1.3–1.7 s JVM and Silicon start (the 128-bit prologue's 4.6 s is the one real cost: three nested calls over 128-bit chains); the max budget (< 10 s) holds everywhere that has a verdict. Nice-to-have items **not met at any width** (T/O in 20 s, `Hn8`): the claims that need the bit relation of a symbolic operand beyond a constant mask or the same operand twice: `and(-1,x)==x` alone, `or(x,0)==x`, `and(x,x)==x`, `xor(xor(a,b),b)==a`, `0 <= and(u,m) <= m`, `or(u,m) >= m`, the 10-op idempotence `and(t,t)==t`, `low_bit` (`0 < and(x, 0-x) <= x`), the popcount loop (`x := and(x, x-1)` strictly decreases) and their twins. Met at 32 bits (claims.py, 1.5–2.2 s): `u & 255 <= 255`, `u & 15 < 16`, `u & 1 == u % 2`, `u & 255 == u % 256`, `(u | 4) & 4 == 4`, `(u & ~4) & 4 == 0`, `xor(u,u)==0`, `and(x,0)==0`, `and(x,y)==and(y,x)`, `xor` range.

**5. Other forms tried (core battery, N=64 unless noted, 20 s timeout; `claims.py` at N=32):**
- Single function per op without hierarchy, bit chain, ite, with a range postcondition (`Ditefp`): the required items pass at 32 and 64 bits (2.1–4.1 s at 32, 3.2–10.3 s at 64, the per-function cost) and fail the budget at 128 (`consts` T/O at 40 s, the others 8–29 s); `Ddiv` (per-bit `(ba+bb) \ 2`) and `Dlin` (bit products) are worse (`Dlin` T/O on everything symbolic at 16 bits, `Ddivf` without postcondition T/O on the signed range at 32).
- Hierarchy with other leaves, with range postconditions (`ensures`), 64 bits: `H4`, `H8`, `H16`, `Hd8` (div per bit), `Hh8` (Horner) all pass the required items; `H4`, `H8`, `Hd8`, `Hh8` time out on at least one must-fail twin (`and_m1_fail`, `false_prop`, `ten_ops_fail`), `H16` and **no-postcondition `Hn8`** do not (all time out on `low_bit_fail`). A postcondition (`ensures lo <= result <= hi`) is therefore no help, and would also need a lint change (`ensures` is a check line, `lint_viper.py:93,372`, and has no map entry in a prelude function).
- 128 bits, core battery: `Hn8` passes everything but `ten_ops_fail`; `Hn4` the same (consts 2.26, prologue 5.20, range_s 2.22); `Hn16` times out on `range_s` (flaky). `Hn8` is the recommendation (fewer levels than `Hn4`, no timeout on a required item).
- Per-bit forms with `or`/`xor` linear in `and` (`Hm8` div, `Hi8` ite, 32 bits): they prove `xor(x,0)==x` or `or(x,0)==x` (10–13 s) but lose some mask claims (`Hm8`: `(u | 4) & 4 == 4`, `u & 255 == u % 256`, `or <= max`; `Hi8`: `xor(u,u)==0`, `(u | 4) & 4 == 4`); `Hn8` has the best coverage of the masks.
- Lemma postconditions (`result <= b % 2^N` on the unsigned `and`; `Hb8`): proved at 32 and 64 bits (`u_and_le_k` 5.4 s, 10.1 s) but the function proofs raise every file to 3–12 s and 64 bits `consts` and 128 bits everything time out: not recommended.

**6. Verdict.** `Hn8` meets the budget for the required battery at 32, 64 and 128 bits with a total, non-recursive, body-only prelude of functions that Silicon never has to prove anything about (no postcondition, no axiom, no `inhale`); closed constants (prologue asserts) are decided by evaluation, including a failing one (exactly 1 error). Shapes it cannot prove symbolically (list in 4) hang rather than fail: **Z3 does not return `unknown` for them**. Silicon's `--assertTimeout 3000` turns each into an ordinary `Assert might fail` in 6–12 s (`Hn8`, 64 bits: `ten_ops_idem` 6.0 s, `xor_invol` 6.0 s, `low_bit` 12.0 s, `low_bit_fail` 6.7 s; `S21_ARGS="--assertTimeout 3000"` for `run.py`), sound but an incompleteness: whether the driver sets it is a decision for 0023. The lint accepts the function form (`lint_viper.py` on `Hn8_128_prologue.vpr`: only the spike methods' missing `decreases` and map are reported, no complaint about a `function`).

**7. The emitted text** (`gen_h.py`, `Hn8`; `vpr/Hn8_128_prologue.vpr` is a whole file). Leaf (8 bits; the chain abbreviated here, the file has all eight `let` pairs; `c_uand_8` has `+ 128 *` instead of `+ (-128) *` on the last term):

```text
function c_and_8(a: Int, b: Int): Int
{
  let qa1 == (a \ 2) in let qb1 == (b \ 2) in let qa2 == (qa1 \ 2) in let qb2 == (qb1 \ 2) in ... let qa8 == (qa7 \ 2) in let qb8 == (qb7 \ 2) in (((a - 2 * qa1) + (b - 2 * qb1) == 2) ? 1 : 0) + 2 * (((qa1 - 2 * qa2) + (qb1 - 2 * qb2) == 2) ? 1 : 0) + 4 * (...) + 8 * (...) + 16 * (...) + 32 * (...) + 64 * (((qa6 - 2 * qa7) + (qb6 - 2 * qb7) == 2) ? 1 : 0) + (-128) * (((qa7 - 2 * qa8) + (qb7 - 2 * qb8) == 2) ? 1 : 0)
}
function c_and_16(a: Int, b: Int): Int { c_uand_8(a, b) + 256 * c_and_8(a \ 256, b \ 256) }
function c_uand_16(a: Int, b: Int): Int { c_uand_8(a, b) + 256 * c_uand_8(a \ 256, b \ 256) }
function c_and_32(a: Int, b: Int): Int { c_uand_16(a, b) + 65536 * c_and_16(a \ 65536, b \ 65536) }
function c_or_32(a: Int, b: Int): Int { c_uor_16(a, b) + 65536 * c_or_16(a \ 65536, b \ 65536) }
function c_xor_32(a: Int, b: Int): Int { c_uxor_16(a, b) + 65536 * c_xor_16(a \ 65536, b \ 65536) }
```
(In the files each function is on several lines, as `prelude.cpp`'s `fn` writes them.) The per-bit test for `or` is `... >= 1` and for `xor` `... == 1`. Names: `c_<op>_N` (signed computed types), `c_u<op>_N` (unsigned), `op` in `and or xor`, `N` in 8, 16, 32, 64, 128 (the lowering needs 32, 64, 128). `~a` stays inline: `-1 - a` (signed), `MAX - a` (unsigned).

**C++ sketch for `src/viper/prelude.cpp`:**

```cpp
struct BitFn { std::string op; int width; bool is_unsigned; };
bool parse_bit_fn(const std::string& n, BitFn& f);   // "c_and_32", "c_uxor_64"; width in {8,16,32,64,128}

// prelude_closure, after the existing c_shr -> c_pow2 rule:
std::function<void(const std::string&, int, bool)> add = [&](const std::string& op, int w, bool uns) {
  if (w > 8) { add(op, w / 2, true); add(op, w / 2, uns); }       // low half unsigned, high half as the parent
  all.insert(bit_fn_name(op, w, uns));
};
for (const std::string& n : std::set<std::string>(used)) { BitFn f; if (parse_bit_fn(n, f)) add(f.op, f.width, f.is_unsigned); }

std::string bit_fn_text(const BitFn& f) {
  if (f.width == 8) {   // leaf: the chain and the weighted sum
    std::string chain, sum;
    for (int k = 1; k <= 8; ++k) {
      auto q = [&](char v, int i) { return i == 0 ? std::string(1, v) : std::string("q") + v + std::to_string(i); };
      chain += "let qa" + std::to_string(k) + " == (" + q('a', k - 1) + " \\ 2) in let qb" + std::to_string(k) + " == (" + q('b', k - 1) + " \\ 2) in ";
    }
    for (int k = 0; k < 8; ++k) {
      std::string ba = "(" + q('a', k) + " - 2 * qa" + std::to_string(k + 1) + ")", bb = /* same for b */;
      std::string t = f.op == "and" ? "((" + ba + " + " + bb + " == 2) ? 1 : 0)"
                    : f.op == "or"  ? "((" + ba + " + " + bb + " >= 1) ? 1 : 0)"
                                    : "((" + ba + " + " + bb + " == 1) ? 1 : 0)";
      std::string w = (k == 7 && !f.is_unsigned) ? "(-128)" : std::to_string(1 << k);
      sum += (k ? " + " : "") + (k == 0 ? t : w + " * " + t);
    }
    return chain + sum;
  }
  int h = f.width / 2; std::string p = cerastes::vir::dec::pow2(h);
  return bit_fn_name(f.op, h, true) + "(a, b) + " + p + " * " + bit_fn_name(f.op, h, f.is_unsigned) + "(a \\ " + p + ", b \\ " + p + ")";
}
// emit in ascending width order (dependency order), only the closure of the used names.
```
(`bit_fn_name(op, w, uns)` is `"c_" + (uns ? "u" : "") + op + "_" + std::to_string(w)`; the sketch is not compiled, the generators in `gen_h.py`/`gen_split.py` are the tested form; `ba`, `bb` use `q('a', k)`/`q('b', k)` the same way.)

**Surprises.** (1) The proposed flat forms prove constants in 2 s but nothing symbolic past 8–12 bits: Z3 proves a symbolic range or identity of a flat bit sum only by case splits. (2) Splitting the work in halves is what makes Silicon's per-function cost linear: a 128-bit flat function costs 6.5 s of Silicon time before any proof, the whole hierarchy 1 s. (3) A range postcondition makes things worse, not better (must-fail twins hang more often). (4) Z3 hangs rather than returning `unknown` on unprovable symbolic bit identities; `--assertTimeout` bounds them. (5) JVM and Silicon start cost 1.3 s, so no file is faster than that. (6) `pkill -f silicon.jar` was run once early in the spike to clean up my own timeouts; the baseline run had already ended (its log says `copied`, 17:03; the kill was at 17:17), but any other Silicon run of another session at that moment would have been killed as well. Later cleanups killed only PIDs of my own orphans.

## M4.4b2: values the front end computes

Probes P42-P44 for the M4.4b2 decision (static assertions, wrapped character literals, array and other layouts in types; record
`0024-front-end-values.md`; the Q32 and Q39 answers of handoff §11). Evidence is under `build/m4.4b2/P42/`, `P43/`, `P44/`
(gitignored: probes, one `.il`/`.err`/`.rd`/`.sa`/`.ver` per file and mode; the large `.il` files of P43/P44 were deleted, regenerate
with each directory's `run*.sh`; summaries `P42.md`, `P43.md`, `P44.md`, `P44/rd_summary.txt`, `P43/blk/scan.txt`, `P42/diag/diag.txt`).
The probes ran with the **frozen pre-spike binaries** (`build/m4.4b2/old-bin/{cerastes-fe,cerastes}`, copies of the build of 2026-10-24,
before any M4.4b2 change); the wrapper is `build/m4.4b2/fe.sh` (`scripts/cerastes-fe.sh` with that binary). `il_def.h` lines are at the pin
`f0ed5525af` (`third_party/edg/src/`).

**Commands**

```text
build/m4.4b2/fe.sh --c23|--c++20 --display_error_number --il_display <file>      IL evidence (probes' .il)
build/m4.4b2/old-bin/cerastes verify <file> --std=c23|c++20                       today's encoder (.ver)
build/m4.4b2/rd.sh --c23|--c++20 <file>      scratch reader (scratch/p42.cpp): every ck_integer constant, expression-node constant and
                                             case entry with character_kind, expr/local_expr_ref, orig_type, enum_value_range
build/m4.4b2/rd44.sh <file>                  scratch reader (scratch/p44.cpp): sizeof/alignof/datasizeof nodes, user typedefs and variables,
                                             types described by the IL fields a helper must follow
python3 build/m4.4b2/tree.py <il> sa|scan    condensed block-scope static assertions: condition with its expr tree / leaf kinds
```

The scratch readers are a replacement `back_end()` linked against snapshot copies of `libedg_fe.a`, the driver objects and the cerastes
libraries (`scratch/lib/`); no file under `src/` or `third_party/` is touched. `--il_display` prints `character_kind` only for strings
(`il_display.c:1392`); a non-`chk_char` constant's one-line summary carries a `L'`/`u'`/`U'`/`u8'` prefix (`il_to_str.c:7077-7095`).

### P42: **PASS: no stop condition for character literals; one stop-rule hit by the letter, the known template-argument class**

**1. Where a wrapped literal can be read** (`P42/probes/lit.c`, `lit.cpp`, `places.c`, `places.cpp`, `enum3.cpp`; `*.rd`):

| shape | C23 | C++20 |
|---|---|---|
| `'\x7f'`, `'\x80'`, `'\xff'`, `'\177'`, `'\200'`, `'\377'` | `int` 127, -128, -1, 127, -128, -1; `expr` NULL, `orig_type` NULL, `character_kind = chk_char` | `char`, the same values, same flags |
| `L'\x7fffffff'`, `L'\x80000000'`, `L'\xffffffff'` | `int` 2147483647, -2147483648, -1, `chk_wchar_t` | `wchar_t` (signed 32-bit), same values, `chk_wchar_t` |
| `u'\xffff'`, `U'\xffffffff'` | `unsigned short` 65535 `chk_char16_t`, `unsigned int` 4294967295 `chk_char32_t` | `char16_t`, `char32_t`, never negative |
| `u8'\xff'` | **`unsigned char` 255, `chk_char`** | `char8_t` 255, `chk_char8_t` |
| the literal converted (`int c = '\xff';`) | a second constant: `expr` set, `orig_type` = the literal's type, `character_kind = chk_char` (lost on this copy; the literal is inside its `expr`) | same |
| `enk_constant` expression node of a literal | `constant.ptr` is the **same `a_constant`** (same address): kind and value survive | same |

**2. Places** (all keep the literal as in the table; the literal is a constant without `expr` and without `orig_type`):
function bodies, `requires`/`ensures` (`E L5:5 -1`), `check`/`invariant` (position of the macro use), a macro `#define NEG '\xff'`, a local
or file-scope `const char K = '\x80'` (C: the variable's constant **is** the `int` literal), a C++ default argument (literal plus the
call site's converted copy), and **`case '\x80':`**: in C the `case_value` is the literal itself (`int -128 expr=N`); in C++ it is
a converted `int -128 expr=Y orig_type=char` copy whose `expr` holds the literal. **Enumerator initializers (the `:1040` constant):
the enumerator's constant is the literal constant with `is_named_constant_definition` set**; C: `int -1 expr=N orig_type=- ck=chk_char`
(`E = '\xff'`), `chk_wchar_t` for `L'..'` in an unfixed enumeration; C++: `type = the enumeration`, same flags; **a fixed underlying type
(`enum : int { C = L'\xffffffff' }`) loses the kind** (`chk_char`, value kept). `enum : unsigned char { U = '\xff' }` is an EDG error #1749.

**3. An enumerator's explicit initializer** is recorded in `source_corresp.decl_pos_info->variant.enum_value_range`
(`il_def.h:1974-1980`, guard `is_enum_constant(c)`, `il.h:2428`; `il_display.c:769`): null for no initializer, a range for every written
one (`enum { A = -2, B = '\xff', C, D = 5, F = -2, G, H = L'\xffffffff', J }`: `B evr=1:20-1:25`, `H evr=1:53-1:65`; `C`, `G`, `J`
`evr=none`). Without it an enumerator with a wrapped literal and an implicit one (`enum { A = -2, B }` / `{ A = -2, B = '\xff' }`)
are the same constant. The stale `end_position` of an implicit enumerator (copied from the previous one) must not be used.

**4. Today:** `int f(void) ensures r == -1 { return '\xff'; }`, `enum { E = '\xff' }; ... return E;` (`r == -1`), `return L'\xffffffff'`
and `char c = '\xff'; return c;` all verify in both modes (`probes/gap.c`).

**5. Census** (`probes/census.{c,cpp}`: 47 shapes, all `K` and `E` lines of both modes): a negative constant with `expr` NULL and
`orig_type` NULL is only (a) a character literal (bare, or inside a same-type cast), (b) an enumerator **without** initializer
(flagged `enumerator=1`, `evr=none`), or (c) a **non-type template argument** (C++: `tp<-1>`, `TS<-5>::v`: a constant with no position and
the template-id's argument node), the class of 0022 item 3 (`BACKING_EXPR_FOR_NONTYPE_TEMPL_ARG` is false). (c) is closed today: a template,
a class and a use of a system template's variable are rejected (`probes/targ*.cpp`, `sysT*.cpp`). Every other negative constant
(`-1`, `-2147483647 - 1`, `INT_MIN`, `SCHAR_MIN`, `(char)-1`, `(signed char)200`, negative enumerators, `case -1`, `~0`, `(int)-3.9`, ...) has an `expr`;
no constant has `expr` NULL and a non-null `orig_type`.

**5b. Census 2 (audit round 1; `P42/c2/`):** the first census missed unflagged negative constants **typed by an enumeration** with no `expr` and no `orig_type`: the
initializer copy of an enumeration-typed object (`enum E y = A;`, `const enum E K = A;`, C++ and C23, unfixed and fixed) and, in C23, `case A:` over a **fixed**
enumeration (`: int`, `: long`, `: short`, `: signed char`, `: long long`, `: char`). A second run over 23 files in both modes (enumerators used in locals of the
enumeration, `int`, `long`, `short` and a typedef, returns, arguments, comparisons, `?:`, casts, `case` over `int` and over the enumeration, contracts, file-scope constants, enumerator
initializers, `static_assert`, C++ default arguments, `enum class`) finds 81 such constants, **all enumeration-typed**, and **none whose type is the plain `int` (C) or `char`/`wchar_t` (C++)**
except the control character literals. So a wrapped literal is identified by value **and** type, and no other shape matches.

**6. Diagnostics** (`P42/diag/diag.txt`): `'\x100'`, `'\400'`, `'\777'`, `L'\x100000000'`, `U'\x100000000'`, `u8'\x100'`, `"\x100"` are **error #27-D**;
`99999999999999999999`, `0xFFFFFFFFFFFFFFFFF`, `18446744073709551616` (and, in C, `18446744073709551615`) **error #23-D**; both exit 2 in `cerastes-fe` and
`cerastes verify`, both modes. The plan's premise (warnings in clang mode) is wrong: `literals.c:1731` chooses `es_warning` only under
`gnu_version_is(any_version)`, which is `gnu_mode && !clang_mode` (`lang_feat.h:1437`); `verify` always runs in clang mode
(`verify_main.cpp:304`, GNU emulation refused `edg_args.cpp:151`). No `--diag_error` entry is needed; `u'\x10000'` (#3255, C23 warning) is
already in 0023's list.

### P43 (block scope, frozen binary): **PASS: every non-literal condition keeps `expr`**

**1. The entity** (`il_def.h:2071-2087` `a_static_assertion {condition, string_literal, position}`; `decls.c:15566-15594`): a block-scope assertion is an
`iek_static_assertion` in the `entities` list of a `stmk_decl` (`il_display`: `kind: stmk_decl`, `entities: ... static-assertion`), in
every nested block, in dead code and after `return` (`misc/n01.c`: 8 written, 8 found). `string_literal` is NULL for the one-argument
form. `condition` is set only when the assertion holds (`decls.c:15660-15668`); a failed one is an EDG error (#1574 C, #2784 C++, exit 2), so a
NULL condition never reaches the back end. A false non-dependent assertion in a **template** is *not* an error (CWG2518,
`decls.c:15649-15655`) and is recorded with value 0 (`misc/t01.cpp`).

**2. The condition constant:**

| shape | C23 | C++20 |
|---|---|---|
| comparison, logic, arithmetic, `sizeof`, casts, local constexpr use | `K<int 1 expr=<written tree>>` | `K<bool 1 expr=<written tree>>` |
| an integer-valued condition (`static_assert(4)`, `sizeof(int)`, `1 + 1`) | **unconverted**: `K<int 4 expr=NONE>`, `K<unsigned long 4UL expr=enk_sizeof[int]>`, `K<int 2 expr=add(1,1)>` | `K<bool 1 expr=eok_bool_cast[bool] {gen}(...) orig_type=int>` |
| lone literal `1`, `true` | `K<int 1 expr=NONE>` | `1`: `bool_cast`; `true`: `K<bool 1 expr=NONE>` |
| lone enumerator `N` | `K<int 5 expr=NONE enumerator>` (also `K<int -1 ... enumerator>`) | `bool_cast(K<enum 5 expr=NONE enumerator>)` |
| lone wrapped `'\xff'` | `K<int -1 expr=NONE>` | `bool_cast(K<char -1 expr=NONE>)` |
| float `1.5` | `K<double ck_float>` (not `ck_integer`) | `bool_cast(K<double ck_float>)` |
| `(1 << 40) == 0`, `2147483647 + 1 < 0` | **accepted**, folded with wrap (`K<int 1 expr=eq(K<int 0 expr=shiftl(1,40)>, 0)>`) | **EDG error #28** (not a constant) |
| `(signed char)200 == -56`, `65536u * 65536u == 0u` | accepted (`expr` keeps the casts / the multiply) | accepted, same trees |
| local `constexpr`/`const` `k` (`static_assert(k == 5)`) | `const` object: `#1901` (error in `verify`; the IL has `K<int 5 expr=var k:int>`) | `K<int 5 expr=var k:int>`: the `expr` is the `enk_variable` itself (as a case value, P24) |

**3. The skip shapes** (`P43/blk/scan.txt`: the leaves of each condition; "system" is `seq_is_in_system_header` of the declared entity's
`decl_position`): a call `eok_call` with an `enk_routine` operand (a user `constexpr` function, `std::numeric_limits<int>::max()`,
C `__builtin_constant_p`); `enk_builtin_operation` (`__is_integral`, `__is_same`, `__builtin_offsetof`, both modes; `builtin_operation.kind`
names it); `enk_concept_id` (`std::integral<int>`); an `enk_variable` declared in a system file (`std::is_integral_v<int>`, `is_integral<int>::value`,
`numeric_limits<int>::digits`); a flagged enumerator constant declared in a system file (C `_CLOCK_REALTIME`). **Types never trigger it**
(`enk_sizeof[std::size_t]`, `enk_alignof[int]`). `noexcept(1 + 1)` is an `eok_noexcept` operation (an unknown kind, rejected by the encoder).
A user `constexpr bool t = std::is_integral_v<int>;` is rejected at its own declaration today (0017 item 2), not skipped. A user `constexpr`
function and a user struct are themselves rejected today (0012), so a user call or user static member does not reach an assertion yet.
Every skip shape **verifies** today (the assertion is not looked at).

### P43 (file scope, new binary): **PASS: no stop condition: no block-scope assertion appears in the file-scope list**

Reader `scratch/sq.cpp` over `il_header.primary_scope->source_sequence_list` and each function scope's `source_sequence_list` /
`src_seq_sublist_list` (`il_def.h:18596-18611`; `a_source_sequence_entry {next, prev, entity}`, `src_seq.h:27-41`); evidence `P43/fs/*.sq`.

- **File list contents:** a file-scope, namespace-scope (flat, with `src-seq-end-of-construct` after each closed namespace), `extern "C"`
  (after a `linkage-spec-block` entry) and **class-scope** (C++ `struct`, nested class, C `struct`) assertion is an `iek_static_assertion` entry of the
  file-scope list, in source order; the class-scope one sits right after the `type` entry, before the `field` entries. The condition constant keeps its
  `expr` (position of `condition->expr` = the condition's text), type `bool` / `int`; a lone `static_assert(true);` has `expr` NULL and no string.
- **Block scope** assertions are on the function scope's list (and are the `stmk_decl` entities); the overlap with the file list is empty in all
  four dumps (`fs.cpp`, `fs.c` in C23 and C++20, `sec.cpp`). A class declared in a function keeps its assertion in a `src-seq-sublist` entry of the function's list
  (also on `src_seq_sublist_list`), in neither the file list nor a `stmk_decl`.
- **System headers:** their file-scope assertions are in the list with `SYS` positions (libc++: 82 in `<array>`, 103 in `<vector>`, 94 in `<string>`, 64 in
  `<memory>`, 2 in `<utility>`; none in `<cstdint>`, `<cmath>` or the C headers tried): filter by `seq_is_in_system_header(position.seq)`.
- **Macros:** an assertion produced by a macro defined in a system header has its `position` and `condition->expr` position **both at the use site in the user file**
  (`FULLY_RESOLVED_MACRO_POSITIONS`), so a position test treats it as the user's; a template's is dependent (`ck_template_param`, no `expr`).
- **Secondary declarations** (`src-seq-secondary-decl` with `decl_position` and `entity.kind`) and `pragma`, `macro`, `using-decl`, `field`, `template` entries sit in the same
  list; assertions between redeclarations remain ordinary entries in source order.

### P44: **PASS: array, `_BitInt`, vector and non-type-template-argument types are reachable only through `sizeof`/`_Alignof`/`__datasizeof`/`typeof`/`decltype` operands and typedefs; declared objects of those types are already rejected**

(`P44/pr/*.rd`, `.ver`; 104 runs, `rd_summary.txt`.)

| shape | IL |
|---|---|
| `sizeof(int[3])`, `_Alignof(char[4])`, `sizeof(char(*)[4])`, `sizeof(int(&)[3])` | `enk_sizeof`/`enk_alignof` with `sizeof_info.is_type`, `variant.type` a `tk_array` (under `tk_pointer`, `is_reference`); **`bound_constant` non-NULL**, `expr` NULL for a literal bound |
| `sizeof(char[65536u * 65536u + 1])` | `tk_array` of **1 element** (`number_of_elements` is the wrapped value), `bound_constant->expr` set; `r == 1` verifies today |
| `sizeof((char[4]){0})`, `sizeof("abc")` | **expression operands**: `sizeof_info.is_type` FALSE, `variant.expr` an `enk_temp_init` / `enk_constant` whose type is a `tk_array` (the string's has no `bound_constant`) |
| `typeof(int[3])`, `typeof(x)`, `decltype(x)`, `decltype((x))`, `decltype("abc")`, `using buf = char[8]` | `tk_typeref`, `variant.typeref.kind` = `trk_is_typeof_with_type_operand`, `trk_is_typeof_with_expression`, `trk_is_decltype`, `trk_is_alias` (typedef: `trk_none`), target `variant.typeref.type` |
| `unsigned _BitInt(8)` | `tk_integer`, `int_kind = ik_unsigned_bit_precise`, width in `variant.integer.extra_info->bit_width` |
| `typedef int v4si __attribute__((vector_size(16)))` | `tk_typeref` (`trk_none`) -> `tk_vector` (`element_type`, `size_constant`) |
| `std::array<char, 4>`, `std::bitset<8>`, `std::integral_constant<int, 3>` | `tk_class`, `variant.class_struct_union.extra_info->template_arg_list` with an entry of `kind == tak_nontype` (`il_def.h:7677-7679`) |
| `typedef char buf[8];` (file and block), `using`, `typedef int inc[];` | a named `tk_typeref` entry (`source_corresp.name`, `decl_position` in the user file), reached by the file-scope walk and by the function-region walk |
| `int (*)(char[3])`, `typedef int fn_t(int)` | `tk_routine`: `return_type`, `param_type_list` (`a_param_type.type`, `declared_type` = before adjustment, `il_def.h:6390-6404`) |
| VLA `sizeof(char[n])` | `tk_array` with `is_vla`, `is_variable_size_array`, `bound_constant` NULL; rejected today as a run-time `enk_sizeof` |
| `_Alignas(8)`, `_Alignas(int)`, `alignas(8)` on a variable | **an attribute** on the variable (`[[_Alignas(<aak_constant>)]]`, `<aak_type>`, `[[alignas(<aak_constant>)]]`); rejected today (0011 item 7's allow-list) in both modes |
| `enum { M = sizeof(char[4]) }` | C: rejected by 0018 item 10 (`reject_enum_list_sizeof_array.c`); C++: verified, `sizeof` of an array |
| `int a[3]`, array/`[*]`/`[]` parameters, `extern int garr[5]`, `char (&g())[4]`, `std::array<char,4> a`, local `typeof(int[3]) *p` | already rejected (`unsupported_type` / `unsupported_declaration`) |

`il_display` does not print the array type entry that only a `sizeof` operand reaches, so the golden tests cannot show `bound_constant`;
the scratch reader reads `a_type` directly.

### P45: the source sequence lists (the spike): **PASS after the walker change the owner chose**

`GENERATE_SOURCE_SEQUENCE_LISTS` 1 in `cmake/EdgFrontEnd.cmake`; EDG then also sets `GENERATE_LINKAGE_SPEC_BLOCKS` 1 and
`GENERATE_MICROSOFT_IF_EXISTS_ENTRIES` 1 (`host_envir.h:1734-1760`; `MICROSOFT_EXTENSIONS_ALLOWED` is 1 in the stock
`cmake_defines.h:302`), and the template-instantiation variants stay 0. All five are asserted in `src/backend/edg_il.h` and
`scripts/check_edg_config.sh` (both binaries print them, `cmd_line.c:8120-8135`). Evidence `build/m4.4b2/P45.md` and
`p45-*.log`. Full rebuild 102 steps; `bootstrap.sh --build-only` is needed afterwards (its EDG-tree stamp hashes
`cmake/EdgFrontEnd.cmake`).

| Step | Result |
|---|---|
| `check_edg_config.sh` | ok for `cerastes-fe` and `cerastes` |
| goldens (`tests/annotations tests/il`), no walker change | 998 passed, **28 failed**: every one `attr [[...]] on <unknown owner>` lines, the copies of attribute entries EDG gives a secondary declaration (`decls.c:22128-22139`; any redeclaration with attributes, not only comma lists) |
| goldens, walker ignoring the attributes reached through `iek_src_seq_secondary_decl` | 1024 passed, **2 failed** (`il/g1/check5_attributes.cpp`, `il/inventory/attr_postfunc.cpp`): the walk now follows `a_src_seq_secondary_decl.declared_type` (`walk_entry.h:3861`), the function type of a redeclaration, which no entity owns, and lists its attributes (`attr [[unreg_ns::a2(...)]] on type <unnamed>`) |
| goldens, walker also ignoring a secondary declaration's `declared_type` (owner, 2026-10-26) | see the session's status entry |
| `walker_oracle.py` / `test_walker_oracle.py` | 134 goldens, 1915 facts, 0 failed / OK |
| full `run_tests.py -j 8` | 4682 passed, 2 failed (the two goldens), 1 skipped; **every `.vpr` byte-identical** to the baseline (no new method, no new rejection) |

### Implications for `encode.cpp` and the record

- **Wrapped character literals.** The literal arm of `constant_value` sees `ck_integer`, `expr` NULL, `orig_type` NULL, negative value, type
  `int` (C) / `char` / `wchar_t` (C++) with `character_kind` `chk_char` or `chk_wchar_t`; take N from the kind (8, 32), or, where it is lost
  (converted copies, fixed-underlying enumerators), from the type, or fail on shape. Because the converted copies carry an `expr` to the
  literal the rule is reached on the literal itself. N does not matter for soundness. **`encode_named_enumerator` (`:1040`)** must tell an
  enumerator without initializer from one with a literal by `decl_pos_info->variant.enum_value_range` (P42 fact 3), not by value.
  Do not apply the rule to non-type template arguments (no `expr`, no `orig_type`, negative, no position: P42 census).
- **Truncated escapes and too-large literals** are errors already (#27-D, #23-D); the plan's `--diag_error` entry for them is unnecessary.
- **Static assertions** (block scope): read the `stmk_decl` entity; encode `condition->expr` (C: with `clause_bool` on any integer type, C++: the
  `eok_bool_cast` root), obligations at their lines, then the `internal` guard; lone shapes (literal, enumerator, wrapped literal) have no `expr`;
  C accepts wrapped constants that C++ rejects (#28), so the encoder is the only check in C; the skip triggers are `eok_call`,
  `enk_builtin_operation`, `enk_concept_id`, and a system-file `enk_variable` or enumerator constant; unknown operation kinds (`eok_noexcept`) are the
  encoder's rejection. A failed assertion never reaches the back end.
- **Types in types.** One recursive helper over `typeref` (all kinds), `pointer`, `routine` (return + `param_type_list`, incl. `declared_type`), `vector`, bit-precise
  `integer`, `array` (reject) and `class` instances with a `tak_nontype` argument, run on `sizeof_info` type operands, on typedef entries
  declared in a verified file, and on expression operands; record 0024 item 6 decides to reject `sizeof("abc")` too (an
  array-typed operand, verified before M4.4b2, pinned by no test) until M4.6. `alignas` needs nothing.

## M4.4c: writes outside the chain, `void` `?:` statements, `-Wall` shapes

Probes P46, P46b and P47 for the M4.4c decision (record `0025-...md`; finding A, Q30, and the macro rule of Q37's R5).
Evidence is under `build/m4.4c/p46/`, `p46b/`, `p47/` (gitignored: probe sources, one `.il`/`.err`/`.tree` per file and mode, the
kept Viper in `p47/vpr/`). All runs used the binaries of `8bff6ba` (`build/cerastes-fe`, `build/cerastes`), frozen while the baseline
ran; `il_def.h` lines are at the pin `f0ed5525af` (`third_party/edg/src/`).

**Commands**

```text
scripts/cerastes-fe.sh --c23|--c++20 --il_display <file>       IL evidence (p46/vq*.{c23,c++20}.il, p46b/mac.*.il)
python3 build/m4.4c/p46/tree.py <il> [fn,fn]                   condensed statement/expression trees (p46/*.tree); no source under src/
python3 build/m4.4c/p46b/eqpos.py <il>                         every eok_eq node with position and orig position of it and its operands
build/m4.4c/p47/run.sh <file...>                               build/cerastes verify in both modes, --keep-vpr=p47/vpr/<name>.<mode>.vpr
```

### P47: finding A **REPRODUCED: the `uninitialized` check of a flagged local that a later write of the same full expression precedes is vacuous**

**Source** (`p47/a1_comma.c`; `f` and `g` are bodiless with `ensures(r, r == arg)`: trusted, the file has no other contract):

```c
int h(int a)
{
  int x;
  int y = (x = (g(f(a) > 0 && x > 0), 2));      // L12: C reads x uninitialized when f(a) > 0
  return y + x;
}
```

Both modes: `verified (1 function; ...)`, exit 0, with no warning from EDG; the Viper is identical but for the `? 1 : 0` truth-value wrappers
(`p47/vpr/a1_comma.c23.vpr`, `.c++20.vpr`). The emitted body, in order (`var` lines and the contract left out):

```text
  d_v_x__1 := false
  v_x__1 := 0
  t__1 := v_x__1                                  the snapshot of `x > 0` (its value, taken where C reads it)
  v_x__1 := 2                                     the write `x = ...`
  d_v_x__1 := true
  t__0 := f_f__2(v_a__0)                          the call f(a), lowered after the write
  assert (((t__0 > 0) ? 1 : 0) != 0) ==> d_v_x__1                 the uninitialized check: d_v_x is already true
  t__2 := f_g__1(((((t__0 > 0) ? 1 : 0) != 0) && (((t__1 > 0) ? 1 : 0) != 0)) ? 1 : 0)
  v_y__2 := 2
```

**Why**: the flagged snapshot's check has the set `{f} ∪ slot ∪ floor` (`lower.cpp` `snapshot` :814-833, `obl` :397-405), but `touch` records
only `slot` (∅) in `last_access`, and the comma's value drops its left operand's calls, so the write gets the set ∅ and is placed before
the call `f`.

**Other shapes that verify (the hole)**, both modes, same mechanism (the write's set misses the guard):

| file | shape (L12) | c23 / c++20 |
|---|---|---|
| `a1_comma.c` | `int y = (x = (g(f(a) > 0 && x > 0), 2));` | verified / verified |
| `c3_and_write.c` | `int y = (((void)(f(a) > 0 && x > 0), 1) && ((x = 1) == 1));` | verified / verified (the write is in a branch; `t__2 := 1 != 0; if (t__2) { v_x := 1; d_v_x := true ... }`, then `t__0 := f(a)`, then the check) |
| `c4_cond_arm.c` | `int y = (x = ((void)(f(a) > 0 ? x : 0), 2));` | verified / verified |

(`c3_and_write.c`: the first form the orchestrator named, with `(x = 1) == 1` instead of a bare `(x = 1)`, because EDG warns #187 on
`&& (x = 1)`. Under `-Wall -Wextra -Werror` Apple clang reports `-Wuninitialized` on `a1_comma.c` and `c3_and_write.c` and is silent on
`c4_cond_arm.c`; GCC 16 is silent on all three. So `a1`/`c3` are error tests with a native warning, `c4` is not.)

**Controls**

| file | what it shows | c23 / c++20 |
|---|---|---|
| `a1_comma_init.c`, `c3_and_write_init.c`, `c4_cond_arm_init.c` | `int x = 0;`: no flag, no check | verified / verified |
| `a1_comma_req.c`, `c3_and_write_req.c`, `c4_cond_arm_req.c` | `requires(a <= 0)`: `f(a) > 0` is false | verified / verified (correct; the same verdict as the hole, so not a discriminating control) |
| `b1_nowrite.c` | `int y = ((void)g(f(a) > 0 && x > 0), 2);` with no later write | `uninitialized` L12 / same: the check is path precise and fires |
| `b1_nowrite_req.c` | the same with `requires(a <= 0)` | verified / verified: path precision (the check is `t__0 > 0 ==> d_x`, `a <= 0` makes it unreachable) |
| `b2_stmt_after.c` | the write in a later statement (`x = y;`) | `uninitialized` L12 / same |
| `c5_uncond.c` | `x = (g(x), 2)`: an unconditional read | `uninitialized` L12 / same |
| `c7_cond_call_arm.c`, `c8_comma_g_in_cond.c`, `d1_nocomma.c`, `d2_nocomma_arm.c` | a guarded read with the write in a branch, a read after the comma's left call, no comma at all | `uninitialized` L12 / same |

The hole therefore needs both a guard with a call in the set (`gdep`) and a comma that drops the left operand's calls; without the comma
(`d1`, `d2`) the write's set contains the guard's call and the order is right.

**Side finding** (`p47/c6_callfree_guard_write.c`, `int y = (x = ((void)(a > 0 && x > 0), 2));`, a call-free guard): C23 reports
`uninitialized` (correct); **C++20 rejects it**: `error #cerastes-unsupported_expression: the comma operator in an annotation, or in an
expression the compiler folded to a constant` (decision 0012's wording). Not a soundness issue (a rejection); a mode difference to record.
`p47/c2_callfree_comma.c` is rejected by `comma_without_effect` in both modes (its left comma operand `((void)..., 1)` has no effect; not a
valid shape for this probe).

**Existing tests with this shape**: none. A scan of every `.c`/`.cpp` under `tests/` and `doc/modules/ROOT/examples/` (1813 files) for a
line that has a call, a guard (`&&`, `||` or `?`) and a write to a local declared without an initializer that is also read on that line
finds only `sequenced_operands.c`, `sequenced_operands_post.c` (`if ((x = a) && x > 5)`: the write precedes the read) and three
`CERASTES_INVARIANT(... && ...) for (i = 0; ...)` loops (`i` is written by the init clause before the invariant): none reads a flagged local
in a guarded operand and writes it later in the same full expression. The comma shapes in `tests/scalar/comma_*.c` and `call_comma_*.c`
(`(x = a, x + b)`, `(m = n - 1, m)`) all write first. So A's fix is expected to change no existing test; `check.sh` is the measurement.

### P46: the IL of a `void` `?:` in statement position **PASS: no stop condition (the extra C++ nodes are around the condition only, and C++ already has them for every `?:`)**

Probes `p46/vq.c`, `vq.cpp` (C23 and C++20 from the same text; `s1`-`s10`), `vq2.*`, `vq3.*` (commas), `vqx.cpp` (`throw`, `return`).
Trees: `p46/vq*.{c23,c++20}.tree`. `il_def.h`: `eok_question` :14237, `eok_comma` :14174, `eok_bool_cast` :13998, `result_is_not_used`
:14963, `eval_left_to_right` :15318, `is_parenthesized` :15075, `position` :15110, the `for` statement's `initialization` and `increment`
:15920-15928.

**1. The shape** (C23; `s1`: `c ? v(x) : w(y);`, `s1` is column 32 of line 4):

```text
stmk_expr @4:32-47                                                (the position range includes the `;`)
  enk_operation eok_question : void result_is_not_used=TRUE  @col 34 range 32-46       (the `?`; the range is condition..last arm)
    enk_variable : int variable=c  @col 32
    enk_operation eok_call : void result_is_not_used=TRUE  @col 37 range 36-39
      enk_routine : void (*)(int) routine=v ; enk_variable : int variable=x
    enk_operation eok_call : void result_is_not_used=TRUE  @col 44 range 43-46
      enk_routine : void (*)(int) routine=w ; enk_variable : int variable=y
```

Operands in order: condition, then-arm, else-arm (`next` chain). The node is `enk_operation`, `operation.kind: eok_question`,
`operation.type_kind: tk_unknown` (as always), **type `void`**, `result_is_not_used: TRUE`. No cast, no `enk_temp_init`, no other node wraps
the whole expression: the `stmk_expr`'s `expr` is the `eok_question`.

**2. C++20 differs from C23 only in** (diff of the two trees with columns removed, `vq.*.tree`):

| C++20 | C23 |
|---|---|
| the condition is wrapped in a `compiler_generated` `eok_bool_cast : bool` (`c`, `a`, `b`); a comparison condition (`f(a) > 0`) is typed `bool`, no wrapper | the condition is the bare `int` variable; `f(a) > 0` is typed `int` |
| calls print `eval_left_to_right=TRUE` | not printed |
| assignments are `is_lvalue` with `returns_lvalue_instead_of_usual_rvalue`, `eval_right_to_left` | not printed |
| a `for` statement is the statement itself | the `for` is wrapped in a `compiler_generated` `stmk_block` (the init scope; `il_def.h:15920-15924`) |

None of these is around a void `?:` or around an arm; the bool wrapper on the condition is what C++ puts on every `?:` (il-inventory D2 and D11).
**The stop rule ("extra C++ nodes around a void `?:`") does not fire**; the encoder's `?:` condition code already peels it for value `?:`.

**3. The shapes** (both modes unless said; "arm" is the `eok_question` operand):

| source | node tree |
|---|---|
| `s1` `c ? v(x) : w(y);` | `stmk_expr` > `eok_question : void` (above) |
| `s2` `c ? v(x) : (void)0;` | the else arm is `eok_cast : void result_is_not_used` over `enk_constant : int 0 result_is_not_used` (the cast is not `compiler_generated`, no `is_parenthesized`; column of the cast is the `(`) |
| `s3` `c ? (void)0 : w(y);` | the then arm is that `eok_cast` |
| `s4` `a ? (b ? v(1) : w(2)) : v(3);` | the inner `eok_question : void`, `is_parenthesized=TRUE` (range 30-44 excludes the parentheses; `position` the inner `?`); no wrapper |
| `s5` `(c ? v(x) : w(y)), x++;` | `stmk_expr` > `eok_comma : int result_is_not_used` (the type is the right operand's) > [`eok_question : void` with `is_parenthesized`, `eok_post_incr : int`] |
| `s6` `for (i = 0; i < n; i = i + 1, c ? v(i) : w(i))` | `stmk_for`'s `increment` is an `eok_comma : void result_is_not_used` > [`eok_assign : int result_is_not_used`, `eok_question : void`]; the `?:` is not parenthesized |
| `s7` `for (...; ...; c ? v(i) : w(i))` | `increment` is the `eok_question : void result_is_not_used` itself |
| `s8` `(void)(c ? v(x) : w(y));` | `stmk_expr` > `eok_cast : void result_is_not_used` (not generated; `position` the `(` of `(void)`) > `eok_question : void is_parenthesized` |
| `s9` `f(a) > 0 ? v(x) : w(y);` | the condition is `eok_gt : int` (C) or `eok_gt : bool` (C++) over `eok_call : int` and the constant 0; the arms are as `s1` |
| `s10` `c ? v(1) : w(2);` | as `s1` with constant arguments |
| `u3` `c ? v(x) : w(y), (void)0;` | as `s5`: the comma's operands are the unparenthesized `eok_question` and the `eok_cast` |
| `u4` `z = 1, c ? v(x) : w(y);` | `eok_comma : void` > [`eok_assign`, `eok_question : void`] |
| C++ `t1` `c ? v(x) : throw 1;` (`vqx.cpp`) | the `eok_question` is still `void`, `result_is_not_used`; the else arm is `enk_throw : int` (with `dynamic_init`, `destructor=NULL`), **no cast**; stays rejected |
| C++ `t2` `return c ? v0() : w0();` in a `void` function | `stmk_return` with `expr` = the `eok_question : void` directly, no wrapper |

**4. A comma whose value is used** (`vq3.*`, `z = ((c ? v(x) : w(y)), 2);` and `return ((c ? v(x) : w(y)), 2);`): the comma is `eok_comma :
int`, **without** `result_is_not_used`, parenthesized; the void `?:` is its left operand and still carries `result_is_not_used: TRUE`
(that flag is set on every void operand). So "the comma's value is used" is decided on the **comma node** (`result_is_not_used`, or a non-`void`
type), not on the `?:`. A discarded comma (`s5`, `u3`, `u4`, `s6`) has `result_is_not_used: TRUE` on itself; its type is the right operand's
(`int` for `s5`, `void` for `u4`/`s6`).

**5. Positions.** `position` of the `?:` is the `?` token; `expr_range` runs from the first token of the condition to the end of the last arm;
`is_parenthesized` is a flag on the `?:` (never a node: `PARENS_IN_IL` is 0). The enclosing `stmk_expr` range ends at the `;`.

**6. Today's diagnostic** (`p46/cur.c`, both modes): `error #cerastes-unsupported_type: type "void" is not supported by "cerastes verify" (decisions
0012 and 0018)`, at the `?`.

### P46b: macro body versus macro argument **PASS: the IL can tell them apart; `written_outside_macros` does not, and it need not**

Probe `p46b/mac.c` (and `mac.cpp`: identical trees modulo node addresses, `p46b/mac.{c23,c++20}.eq`), compilers in `p46b/compilers.txt`.
`a_source_position` (`basics.h:896-923`): `seq`/`column` are the start of the **top-level macro invocation**; `orig_seq`/`orig_column` are the
origin of the token: a macro **definition** (body text) or the **argument text** of the invocation. `il_display` prints the `orig` pair only
when it differs from `(seq, column)`. The seq numbers are file-wide (the probe's line 8 is seq 2988 after the headers).

| shape (`mac.c`) | operator `==` position (seq, col, orig) | operands (orig) | where the `==` came from |
|---|---|---|---|
| m1 `SAME(x)`, `#define SAME(v) ((v) == (v))` | 2988 / 24 / (2983, 22) | (2988, 29), (2988, 29) | **body** (orig_seq < seq: the `#define` line) |
| m2 `ID(x == x)` | 2989 / 24 / (2989, 29) | (2989, 27), (2989, 32) | **argument** (orig_seq == seq, columns differ) |
| m3 `assert(x == x)` | 2990 / 17 / (2990, 26) | (2990, 24), (2990, 29) | argument |
| m4 `CERASTES_REQUIRES(x == x)` (file-scope `attribute-arg`) | 2992 / 5 / (2992, 25) | (2992, 23), (2992, 28); **`enk_param_ref` (param 1), not `enk_variable`** | argument |
| m5 `[[cerastes::requires(x == x)]]` written out | 2997 / 28 / plain | 2997 / 26 and 31, plain; `enk_param_ref` | written: not in a macro |
| m6 `EQ(x, x)`, `#define EQ(a, b) ((a) == (b))` | 3001 / 24 / (2985, 23) | (3001, 27), (3001, 30) | body |
| m7 `x == x` | 3002 / 26 / plain | plain, plain | written |
| m8 `SAME(` newline `x)` | 3003 / 24 / (2983, 22) | (3004, 5), (3004, 5) | body; the operands are argument text on the **next line**: orig_seq > seq |
| m9 `ID(x) == x` | 3006 / 30 / plain | left (3006, 27) argument; right plain | written; mixed operands |
| m10 `ID(ID(x == x))` | 3007 / 25 / (3007, 33) | (3007, 31), (3007, 36) | argument (nested wrappers resolve to the user's text) |
| m11 `ID(SAME(x))` | 3008 / 25 / (2983, 22) | (3008, 33), (3008, 33) | body |
| m12 `OUT(x)`, `#define OUT(a) EQ(a, a)` | 3009 / 25 / (2985, 23) | (3009, 29), (3009, 29) | body (of `EQ`, the nested macro) |
| m13 `ID(x) == ID(x)` | 3010 / 31 / plain | (3010, 28), (3010, 37) arguments | written; both operands arguments |

`is_parenthesized` is `TRUE` on the operator and operands whose parentheses came from a macro body (m1, m6, m8, m11, m12) and on m3
(`assert`'s own parentheses), `FALSE`/absent where the user wrote none (m2, m4, m5, m7, m9, m10, m13).

**Answer.** The IL tells a token written in a **macro body** from one written in a **macro argument**: with `(s, c)` its `position` and
`(os, oc)` its `orig` pair, a body token has `os < s` (the `#define` line comes before the use; `os == 0` for predefined and command-line
macros) and an argument token has `os >= s` with `(os, oc) != (s, c)` (`os > s` when the argument is on a later line of the invocation,
m8). A token written outside every macro has `(os, oc) == (s, c)`.

**What `written_outside_macros` (`encode.cpp:2117`) decides** (`position.orig_seq == seq && orig_column == column`, per node; the rule of
`check_self_assignment` is "the operator and both operands pass it"): fires (the three tokens plain) **only** for m5 and m7; exempt for all of
m1-m4, m6, m8-m13 (m9 and m13: the operator is plain but an operand is not). It does not separate m2-m4, m10 (the user wrote
`x == x` in an argument) from m1, m6, m8, m11, m12 (the operator comes from a body).

**What the compilers do** (`p46b/compilers.txt`; `-Wall -Wextra`, Apple clang and GCC 16, both modes; the attribute lines are unknown-attribute
noise): **clang warns only m7** (`-Wtautological-compare`); **GCC warns m7, m9 (`ID(x) == x`) and m13 (`ID(x) == ID(x)`)**, i.e. when the `==` token
is plain even if an operand is an argument token. Neither warns on m1-m6, m8, m10-m12. So the union rule is "the operator token is plain": a
detector that tests the operator alone matches GCC (m7, m9, m13) and is a superset of clang's. A detector that tests all three matches clang
only. `CERASTES_REQUIRES(x == x)` (m4) is argument text with a non-plain operator: both compilers are silent (they do not even see the
comparison, the attribute is unknown to them), so under either rule it is accepted; the written `[[cerastes::requires(x == x)]]` (m5) is plain
and would be rejected by a rule that applies to contract expressions. Whether to reject a self-compare of a contract parameter at all is not
something a compiler decides; it is the owner's call under R5.

**The rule if the 0021 item 8 simplification is not wanted**: "operator plain" (GCC's), or "operator and both operands have `os >= s`"
(rejects the m2-m4 and m10 shapes whose text the user wrote, accepts body-written ones); both are expressible with the position pair alone.

**Surprise.** The operands of a contract expression are `enk_param_ref` nodes (`param_ref.param_num`, `levels_up`), not `enk_variable`
(m4, m5): a "same variable on both sides" test for annotations compares `param_num`/`levels_up` pairs, not `variable.ptr`.

### P48: the compiler matrix of the `-Wall` shapes (record 0025 item 6): **PASS: no shape is `-Wextra`-only; 8 rules cover every warned shape**

Evidence: `build/m4.4c/p48/` (gitignored: `gen.py` regenerates `matrix.md`, `matrix-r7-full.md` and `raw/` in about 90 s; `r1.c` … `r8.c`, `misc.c` and `.cpp` copies). The summary follows as written by the probe.


Method: `gen.py` writes one file per rule (`r1.c` ... `r8.c`, `misc.c`; identical `.cpp` copies), one shape per line, and compiles each
with Apple clang 21.0.0 and GCC 16.2.0, C `-std=c23` and C++ `-std=c++20`, at three tiers (none, `-Wall`, `-Wall -Wextra`),
then runs `build/cerastes verify --std=c23|c++20`. No shape warns only at the `-Wextra` tier (no `E` cell anywhere): the union rule
is exactly "warns with no option, or with `-Wall`". Unused-parameter/variable warnings are filtered. clang needs `SDKROOT` for
`<assert.h>` when run by its real path.

Order of the four letters in a cell: clang C, clang C++, gcc C, gcc C++.

#### What cerastes would reject (some compiler warns at tier D or A), per rule

* **R1** all four shapes (`a && b || c`, `a || b && c`, `a || b && c || d`, `a && b || c && d`): all four compilers, `-Wall`
  (`-Wlogical-op-parentheses` clang, `-Wparentheses` gcc). Silent everywhere: `(a && b) || c`, `a && (b || c)`, `a && b && c`,
  `a || b || c`, `a || (b && c)`. As planned.
* **R2** `a + b << c`, `a - b << c`, `a << b + c`, `a + b >> c`, `a >> b - c`, `a >> b + c`, `a << b - c`, and the same with unsigned operands:
  all four compilers (clang by default, `-Wshift-op-parentheses`; gcc `-Wall`, `-Wparentheses`). `>>` and unsigned operands warn, as planned.
  Silent everywhere: `(a + b) << c`, `(a + b) >> c`, `a * b << c`, `u * v << w`, `a << b * c`, `-a << b`, `a << b << c`, `a >> b >> c`, `a << (b + c)`.
* **R3** `!x == y`, `!x != y`, `!x < y`, `!x > y`, `!x <= y`, `!x == 1`: all four (`-Wlogical-not-parentheses`; clang default, gcc `-Wall`).
  `!x == 0` warns only in clang (default), not gcc. Silent: `x == !y`, `!x == !y`, `!x == (y > 0)`, `!x == b` (bool b), `!(x == y)`, `(!x) == y`.
* **R4** `a < b < c`, `a > b > c`, `a <= b <= c`: all four; in clang they are an **error by default** (`-Wparentheses`, "chained comparison 'X < Y < Z'
  does not behave the same as a mathematical expression"), in gcc `-Wall`. `a == b == c`, `a != b != c`, `a < b == 1`, `a == b < c`,
  `a < b != 0`, `a < b == c < d` warn **only in gcc** (`-Wall` `-Wparentheses`, both modes); clang is silent on all six. Silent: `(a < b) < c`,
  `(a == b) == c`, `(a < b) == 1`, `a < (b < c)`, `(a < b) == (c < d)`.
* **R5** all six comparison operators on the same variable, for int, unsigned and bool: clang by default and gcc with `-Wall`
  (`-Wtautological-compare`), both modes, parameter, local and global alike. `(x) == (x)` too; `(x) == x` warns in clang (both modes) and gcc C but not gcc C++.
  `x + 1 == x + 1`, `x * 2 == x * 2`, `-x == -x` warn **only in gcc** (both modes, `-Wall`); clang silent. Silent everywhere:
  `(int)x == x`, `(long)x == x`, `x == (int)x` (an explicit cast exempts), `x == l` with `int l = x`, `x + 1 == x + 2`, `h(x) == h(x)`.
  **Macro rule, measured:** `SAME(x)` (both operands from the macro body), `ID(x == x)` (the whole comparison inside a macro argument),
  `assert(x == x)` and `assert(x != x)` are **silent in all four**. But `ID(x) == x`, `x == ID(x)`, `ID(x) == ID(x)` and
  `X == x` (object-like macro, operator written outside the macro) warn in **gcc C only** (`-Wall`); clang C/C++ and gcc C++ are silent.
  So "the comparison operator token came from a macro" exempts everywhere; "an operand came from a macro" does not (gcc C). Whether the IL can
  tell these apart was not measured here (this probe never touched the IL; il-inventory.md:2705 says macro-argument tokens carry orig_seq != seq).
* **R6** `(a > b)` against a constant outside {0,1}-distinguishing values: all four warn for `== 2`, `!= 2`, `== -1`, `!= -1`, `== 3`, `< 2`, `>= 0`, `<= 1`,
  `< 0`, `> 1`, `2 == (a > b)`, `== 2u`, `(a && b) == 2`, `(a || b) == 2`, `(a == b) == 2`, `!a == 2`, and for a bool `p`: `p >= 0`, `p == 2`, `p < 2`
  (clang default `-Wtautological-constant-out-of-range-compare` / `-Wtautological-constant-compare`; gcc `-Wall` `-Wbool-compare`).
  The bitwise forms are **clang-only** (default): `(p ^ q) == 2`, `(p | q) == 2`, `(p & q) == -1`, `(p & q) >= 0` (gcc silent).
  `(a > b ? true : false) == 2` warns in clang (both) and gcc C++ but not gcc C. `(int)(a > b) == 2` warns **only in gcc C** (`-Wbool-compare`).
  Silent everywhere: `(a > b) == 1`, `== 0`, `>= 1`, `< 1`, `> 0`, `<= 0`, `!= 0`, `!= 1`, `(a && b) == c`, `p == 1`, `p == q`, `(a > b) == p`.
* **R7** signed `<<` in a truth context: all four, `-Wall` `-Wint-in-bool-context`, in **every** context tried (if, while, for, do-while, `!`, `&&`/`||` operand
  either side, `?:` condition, `bool r =`, `_Bool r =`, return from a `bool` function, argument of a `bool` parameter), parenthesized or not.
  **Unsigned `<<` is silent** in all four (`u << v`, `u << 1`). `a << 1` warns (constant right operand does not exempt). `a >> b`, `a << b != 0`, `a + b`, `a` silent.
  `a * b`, `x * 2`, `u * v` (unsigned too), parenthesized or not: **gcc only** (both modes, `-Wall`); clang silent in both modes.
  `?:` with constant arms: `c ? 2 : 3` warns in all four (clang **by default**, `-Wtautological-constant-compare`, gcc `-Wall` `-Wint-in-bool-context`);
  `c ? 0 : 3`, `c ? x : 3`, `c ? 3 : x`, `c ? 2 : 0`: gcc only. `c ? 1 : 0` and `c ? 0 : 1`: silent everywhere.
  Through `?:` arms: `c ? a << b : x`, `c ? x : a << b`, `c ? a << b : a << b`: clang C++ in every truth context, clang C only for conversions to bool
  (`bool r =`, `_Bool r =`, bool return, bool argument), gcc silent (gcc C warns only for the both-arms `<<` form in bool conversions). `c ? a * b : x` and
  `c ? x : a * b` silent everywhere; `c ? a * b : a * b` warns gcc C only, only in bool conversions. `c ? u << v : u` silent.
  Through a comma right operand `(x = 1, a << b)`: gcc C++ in every context; clang C only for conversions to bool; `(x = 1, a * b)`: gcc C++ only; `(x = 1, a)` silent.
  Enumerators (unscoped, `enum e7 { Z7, ONE7, TWO7 = 2 }`): `if (TWO7)`, `!TWO7`, `bool r = TWO7` warn in **gcc C++ only** (`-Wint-in-bool-context`);
  `TWO7 && c` warns in clang C, clang C++ and gcc C++; `if (c ? TWO7 : ONE7)` warns in gcc C and C++. `if (ONE7)`, `if (Z7)`, `if (ev)` (enum variable),
  `c ? TWO7 : ONE7` as a value, `if (2)`: silent in all four. So the enum shapes disagree by mode and by compiler (a C-mode rule would need clang+gcc C: only `TWO7 && c` and `if (c ? TWO7 : ONE7)`).
* **R8** (enumerators of two different enumerations): in **C no compiler warns** for `RED + NA`, `RED - NA`, `RED * NA`, `RED / NB`, `RED % NB`, `c + n`, `x ? RED : NA`, `x ? c : n`, `RED & NA`, `RED | NA`
  (clang C, gcc C: silent at every tier). In **C++20** clang and gcc both warn **by default** (`-Wdeprecated-enum-enum-conversion` for the arithmetic and bitwise forms,
  `-Wdeprecated-enum-compare-conditional` for the `?:` forms). Comparisons `RED == NA`, `c == n`, `c < n` warn in all four (`-Wenum-compare`; clang default, gcc C `-Wall`,
  gcc C++ default as `-Wdeprecated-enum-compare`). Silent everywhere: `RED + GREEN`, `(int)RED + NA`, `x ? RED : GREEN`, `x ? (int)RED : NA`, `c + 1`, `c + c`, `c == GREEN`, `(int)c + (int)n`.
  EDG today: rejects (`unsupported_expression`) every enum-variable form `c + n`, `c + 1`, `c + c`, `c == n`, `x ? c : n`, `RED & NA`, `RED | NA`; accepts `RED + NA` and `x ? RED : NA` in both modes;
  `x ? RED : GREEN` is rejected in c++20 only and accepted in c23 (a mode difference that exists today).

#### Surprises versus the plan's R-list

1. **R8 is C++-only.** No compiler warns about `RED + NA` or `x ? RED : NA` in C; both warn by default in C++20 (deprecated). A rule rejecting them in both modes
   (the owner's Q37 answer for `x ? RED : NA`) goes beyond the union rule in C mode; it needs an explicit statement (the "reject like the strictest compiler" principle still applies
   across compilers, but here the difference is the language mode: C has no warning at all).
2. **R5 macro exemption is finer than "any macro".** Comparison written inside a macro (body or argument) is silent in all four; operands from a macro with the operator outside warn in gcc C.
   The identical-expression forms (`x + 1 == x + 1`) are gcc-only, both modes.
3. **R4's equality chains and `a < b == 1` are gcc-only**; clang warns only on the relational chain (an error by default). Mode-independent.
4. **R3 `!x == 0`** is clang-only (gcc silent).
5. **R6**: the bitwise forms (`(p ^ q) == 2` ...) are clang-only; `(int)(a > b) == 2` is gcc-C-only; `(a > b ? true : false) == 2` is not warned by gcc C. Plan's cast extension (`(int)(a > b) == 2`) therefore warns in one compiler in one mode only.
6. **R7**: signed-only `<<` confirmed in all four (no `>>`); `*` is **gcc-only** (including unsigned `u * v`), constant-arm `?:` other than 0/1 is warned (clang by default for `c ? 2 : 3`, gcc for any constant arm that is not
   0/1 including `c ? x : 3`). Warnings through `?:` arms and comma are partial: `<<` arms warn in clang (C++ always, C in bool conversions); `*` arms only gcc C with both arms `*`. Enumerator conditions are gcc-C++-mostly (see R7).
7. **No `-Wextra`-only shape** among those measured, so the union rule never reaches `-Wextra` for these shapes (`-Wsign-compare` appeared on a mixed signed/unsigned near miss and was removed).
8. **Silent and safe to keep accepted in all four (Q30/Q31 pins):** `c ? v(x) : w(y);`, `c ? v(x) : (void)0;`, `c ? (void)0 : v(x);`, `(c ? v(x) : w(y)), x++;`, `x++, c ? v(x) : w(y);`, a nested void `?:`, a void `?:` in a `for` step,
   `y = 0; c && (y = 2); return y;`, `c || (y = 1);`, `c && ((y = 2));`, `c && (y = 2); return y;` with y a parameter, `c ? (y = 1) : (y = 2);`, `if (c) y = 2;` -- no warning at any tier in clang C/C++ or gcc C/C++.
   **EDG, by contrast, warns `#187-D` ("use of "=" where "==" may have been intended") on `c && (y = 2)`, `c || (y = 1)` and `c && ((y = 2))` in both modes** (not on the `?:` form, not on `if (c) y = 2;`) -- that is the warning
   a `frontend-warning assign_where_compare_meant` line would list. The other EDG warnings seen today: `#186-D` on `p >= 0` (bool compared with zero, c23 only), `#111-D` ("statement is unreachable") on `if (ONE7)` and `if (2)` (both modes). Nothing else: EDG warns on none of the R1-R4, R5, R8 shapes.

Table legend: D = warns with no `-W` option, A = first warns with `-Wall`, `!` = an error by default. `REJ:` in an EDG column is a cerastes rejection on that line (informational; calls without contracts in these probes are rejected as `call_without_contract` etc.).


### P49: the region walk lists every block-scope static assertion (record 0025 item 5): **PASS in both modes; one C++ shape EDG itself refuses**

Probes `build/m4.4c/p49/sa.c`, `sa.cpp` (nested block, loop body, `switch` body, `if` and `else` branches, dead code after `return`),
`sa_goto.c`, `sa_goto.cpp` (code after a forward `goto`), `sa_for.c`, `sa_for.cpp` (a static assertion as the init clause of a `for`). The C++
files are the C files with `_Static_assert` spelled `static_assert`. Run with the current build (`build/cerastes verify <file> --std=c23|c++20`),
without and with `--cerastes_verify_selftest=drop_block_static_assertion`, the hook that makes the encoder ignore block-scope assertions
(`encode.cpp` `encode_block_static_assertion`, first line): the cross-check after encoding must then find each assertion that the region walk
(`il_query.cpp` `collect_entities`, `region_static_assertions`, :96-101) lists and the encoder never saw.

| shape (line in `sa.c` / `sa.cpp`) | without the hook (C23, C++20) | with the hook (C23, C++20) |
|---|---|---|
| a nested block `{ _Static_assert(1, ...); s = 1; }` (L8) | verified | `error #cerastes-unsupported_declaration` at L8, "a static assertion this version does not check (decision 0024 item 2)" |
| a `while` body (L15) | verified | rejected at L15 |
| a `switch` body, after `case 1:` (L20) | verified | rejected at L20 |
| the `then` block of an `if` (L27) | verified | rejected at L27 |
| the `else` block (L30) | verified | rejected at L30 |
| dead code after `return s;` (L34) | verified | rejected at L34 |
| after a forward `goto done;` over a block (`sa_goto.*` L9; EDG prints warning #111 "statement is unreachable" at L10 in both runs) | verified | rejected at L9 |
| the init clause of a `for` (`sa_for.c` L9) | C23: verified | C23: rejected at L9 |
| the init clause of a `for` (`sa_for.cpp` L9) | **EDG error #2847 "this declaration is not valid here"** (before cerastes sees it) | the same: not reachable in C++ |

All six assertions of `sa.c` and `sa.cpp` come back in one run, each at its own line, in source order: the region walk lists every block-scope
assertion whatever the statement structure, including those in dead code and after a `goto`. Without the hook every file verifies (the forward
`goto` is legal for cerastes, 0015 item 1: it jumps outward and forward over a block without a declaration; `static_assert` is not an object
declaration). In C++ a static assertion in a `for` init is an EDG error, so that shape cannot reach the cross-check; in C23 EDG accepts
`for (_Static_assert(...); ...)`.

### P50b: dangling `else` and empty bodies in the IL **PASS: both are decidable from the IL, in C23 and C++20, with no macro or `)` position needed**

Probes `build/m4.4c/p50b/de.c`/`de.cpp` (`d1`-`d16`), `eb.c`/`eb.cpp` (`e1`-`e18`), `em.c`/`em.cpp` (macros). Trees `*.{c23,c++20}.tree`
(`python3 build/m4.4c/p50b/tree2.py <il> [fn,...]`: statements with `pos=seq:col-seq:col`, `else_pos`, `COMPILER_GENERATED`), compilers in
`de-compilers.txt`, `eb-compilers.txt` (Apple clang and GCC 16, `-Wall -Wextra`, both modes; the macro probe `em.c` is in the text below).
`il_def.h` (pin `f0ed5525af`): `a_statement.compiler_generated` :16564, `if_stmt.then_statement`/`else_statement`/`else_position` :16630-16640
(`else_position` is "the position of the `else` keyword", `EXTRA_SOURCE_POSITIONS_IN_IL`), `loop_statement` :16652, `body_statement` :16718,
`stmk_empty` :15860.

**Facts common to both questions.**
1. `then_statement` and `else_statement` are always a `stmk_block`. If the user wrote **no braces**, it is a block with `compiler_generated: TRUE`, no
   position (`pos=-`) and the one statement inside (`stmk_if`, `stmk_expr`, `stmk_empty`, ...). If the user wrote **braces**, it is a
   `stmk_block` **without** `compiler_generated`, with `position` (the `{`), `end_position` and `final_position` (the `}`), possibly with no
   statements (`{}`). So the flag that says "written without braces" is `compiler_generated` of the body block (`d1` vs `d2`, `e1` vs `e7`;
   `d6`: then-block written, else-block generated). The statement inside is **never** itself the written block.
2. Compiler-generated blocks nest: C23 wraps every `if`, `while`, `for` and each of their bodies in one or two generated blocks (P46); C++20 wraps
   bodies only (`d1` C23: `stmk_block(gen) > stmk_if > then: stmk_block(gen) > stmk_block(gen) > stmk_if`; C++20: `stmk_if > then: stmk_block(gen) > stmk_if`).
   So a matcher must **peel** generated blocks, never count them. Peel: while the statement is a `stmk_block` with `compiler_generated` and has
   statements, take its **last** statement (one child, except after a label: `[stmk_label, stmk_block(gen) > stmk_if]`, `d16`).
3. Statement fields: `stmk_if.then_statement/else_statement`, `stmk_while.loop_statement`, `stmk_end_test_while.loop_statement` (`do`),
   `stmk_for.statement`, `stmk_switch.body_statement`. The `if`'s `else_statement` is NULL when there is no `else` (the dump prints no line), and
   `else_position` is present only with an `else`.

**1. Dangling `else`** (`d1`-`d16`; `else_pos` = `else_position`, as `seq:column`).

What the compilers warn about (`de-compilers.txt`, line = function number):

| shape | clang | GCC |
|---|---|---|
| `d1` `if (a) if (b) x=1; else x=2;` | warn at the `else` | warn |
| `d2` `if (a) { if (b) x=1; else x=2; }` | silent | silent |
| `d3` `if (a) x=1; else if (b) x=2; else x=3;` | silent | silent |
| `d4` `if (a) while (b) if (c) x=1; else x=2;`, `d5` with `for` | warn | warn |
| `d6` `if (a) { x=1; } else x=2;`, `d7` no `else` at all, `d9` `if (a) if (b) x=1; else x=2; else x=3;`, `d11` | silent | silent |
| `d8` the same as `d1` on several lines (`else` at 11:5) | warn at 11:5 | warn |
| `d10` `if (a) if (b) x=1; else if (c) x=2;` | warn | warn |
| `d12` `if (a) if (b) { x=1; } else x=2;`, `d13` `... else { x=2; }` | warn | warn |
| `d14` `if (a) switch (b) { default: if (a) x=1; else x=2; }`, `d15` with `do` | silent | silent |
| `d16` `if (a) L: if (b) x=1; else x=2;` | silent | **warn** |

**The test** (same in both modes): an `stmk_if` **O** with `else_statement == NULL` whose `then_statement`, followed down through
`compiler_generated` blocks (peeling, last statement), `stmk_while.loop_statement`, `stmk_for.statement` and a `stmk_label` (the statement after
the label, which sits next to it in the generated block), reaches an `stmk_if` **I** with `else_statement != NULL`. Report at `I.else_position`
(`d1`: 1:53 = clang's column; `d4`: 4:70; `d5`: 5:70; `d8`: 11:5; `d16`: 21:57). Stop (accept) at a written `stmk_block` (no
`compiler_generated`: `d2`, `d11`), at `stmk_switch` and `stmk_end_test_while` (`d14`, `d15`), and when O has an `else` (`d3`, `d6`, `d9`).
GCC's label case is included because the rule is the union. The IL for each shape: `d1` O = `stmk_if` pos 1:32 (no `else_pos`), I = `stmk_if` 1:39
with `else_pos 1:53`; `d2` the then of O is a written `stmk_block pos=2:39-2:67`; `d4` O's then peels to `stmk_while`, whose `loop_statement` peels
to I (`else_pos 4:70`); `d9` O has `else_statement`.
Nothing here needs the `if` keyword's or the closing parenthesis's position. Not probed: whether a macro-expanded `else` (its `else_position` has the
`orig_seq`/`orig_column` pair like any `a_source_position`) is silent in the compilers; the union rule would report it.

**2. Empty bodies** (`e1`-`e18`, `em.c`). The null statement is `stmk_empty`, with its own `position` = `end_position` = the `;`
(`e1` 1:28, `e2` 3:5 on the next line, `e15` 18:5 with the `)` on line 18 and the condition ending on line 17). The body is the one statement of
the generated block; `{ }` and `{ ; }` are written blocks (`e7`, `e9`, `e11`, `e12`, `e17`). `stmk_empty` carries `is_fallthrough_statement` for
`[[fallthrough]];` (not a body).

What the compilers warn about (`eb-compilers.txt`; GCC's `-Wempty-body` is in `-Wextra`, clang's too):

| shape | clang | GCC |
|---|---|---|
| `e1` `if (x > 0);`, `e14` with a comment between, `e15` `)` on a later line than the condition | warn at the `;` | warn |
| `e2` `if (x > 0)` newline `;` (the `;` on its own line) | silent | **warn** |
| `e3` `if (x) ; else x = 1;` | silent | silent |
| `e10` `if (x) x = 1; else ;` | silent | **warn** (at the `;`, 12:38) |
| `e18` `if (x) x = 1; else if (x > 2) ;` | warn | warn |
| `e4` `while (x--);`, `e5` `for (...);`, `e13` `for (;x;) ;`, `e16` `while (x > 0)` newline `;` | silent | silent |
| `e6` `switch (x);` | **warn** | silent |
| `e8` `do ; while (x--);` | silent | **warn** (at the `;`, 10:21) |
| `e7` `if (x) {}`, `e9` `{ ; }`, `e11`, `e12`, `e17` | silent | silent |
| `em.c` `if (x) SEMI` (`#define SEMI ;`), `IFX(x)` (`#define IFX(c) if (c);`), `if (x) NOP;` (`#define NOP`) | silent (all three) | **warn** (all three: at the `;` of the macro body, 1:14 and 2:22, and the written `;`, 6:28) |

**The test** (both modes): the body, peeled through `compiler_generated` blocks, is `stmk_empty`, for
(a) `stmk_if.then_statement` when `else_statement == NULL` (`e1`, `e2`, `e14`, `e15`, `e18`'s inner `if`; with an `else` it is accepted, `e3`);
(b) `stmk_if.else_statement` (`e10`; any `then`);
(c) `stmk_end_test_while.loop_statement` (`e8`);
(d) `stmk_switch.body_statement` (`e6`).
Report at the `stmk_empty` `position` (the `;`: `e1` 1:28, `e2` 3:5, `e10` 12:38, `e8` 10:21, `e6` 8:28, as the compilers report). Not rejected:
`stmk_while`/`stmk_for` bodies (`e4`, `e5`, `e13`, `e16`) and any written block (`{}`, `{ ; }`).

**What the IL cannot do, and why it does not matter.** It does not keep the position of the `)` of the condition: `expr_range` is that of the
condition expression (it excludes the parentheses, and a parenthesized `((x))` is a flag, P46), and the statement `position` is the `if`
keyword. A rule "the `;` is on the same line as the `)`" (clang's) is therefore not decidable exactly (`e15` has the condition on line 17, the
`)` and `;` on line 18). The union rule does not need it: GCC warns on `e2` where clang is silent, so **every** null `if` body without an `else`
is rejected, whatever the line. Macros: the `;` of `if (x) SEMI` has `position` 4:25 with `orig` (1, 14), the macro body's, as for any
token (P36); GCC warns on all three macro shapes and clang on none, so the union rejects them too and no `orig_seq` test is needed. Any
macro exemption would have to be an explicit choice (the `;` of a macro body has `orig_seq < seq`, P46b).

### P50: the compiler matrix of the five shape families added during M4.4c (record 0025 item 6, R9–R13)

Evidence: `build/m4.4c/p50/` (gitignored; `gen.py` regenerates `matrix.md` and `raw/`). The summary follows as written by the probe.


Method as P48 (`gen.py`; raw outputs in `raw/`; `matrix.md` has one table per rule; cell order clang C, clang C++, gcc C, gcc C++;
D = default, A = `-Wall`, E = only `-Wall -Wextra`, `-` silent). `scan.py` / `scan_contracts.py` ran clang over every test and doc example.
The EDG column of matrix.md was measured with the binary built after R1-R8 (so R7/R8 rejections appear there); EDG warns on none of the R9-R13 shapes.

#### R9 constant logical operand (clang only: -Wconstant-logical-operand, default; gcc silent at every tier)
Warns (clang C and C++, by default): `x && 4`, `x || 2`, `x && -1`, `x && K` (const int K = 4; in C too), `x && (2 + 2)`, `x && 'a'`, `x && 4u`,
`(x && 4) || y`, `x && (y || 4)`, as a value and as a condition alike. Also clang -Wliteral-conversion on `x && 3.0` (floats are rejected anyway).
Only the RIGHT operand counts: `4 && x`, `2 || x` are silent everywhere. `b && 4` (bool b) silent. `x && true`, `x && !4`, `x && (4 != 0)`, `x && 2 > 1`,
`x && (y == 4)`, `x && y` silent.
Surprise: `x && 1`, `x && 0`, `x || 1`, `x || 0` (and `x && ONE9`, an enumerator of value 1) warn in clang **C++ only** (silent in C), by default. Under the union rule they are rejected in both modes.
`x && TWO9` (enumerator 2) warns as -Wint-in-bool-context (clang both modes, gcc C++; already rejected by R7).

#### R10 out-of-range constant comparison
clang by default (-Wtautological-constant-out-of-range-compare), both modes: the constant is outside the range of the OTHER operand's type, for ==, != and the
ordering operators: `uc == 300`, `uc != 256`, `uc < 256`, `uc == -1`, `sc > 200`, `sc < -129`, `sc == 128`, `us < 70000`, `us == 65536`, `x < 3000000000LL`,
`x == 4294967296LL`, `x == 4294967295LL`, `u == 4294967296ULL`, `(unsigned char)(uc + 1) == 300`, `b == 2` (already R6), and `uc == 300` inside a `?:`.
gcc warns the same shapes only with -Wextra (-Wtype-limits, E): stays accepted by the union rule, but clang's D makes these rejected.
Silent in clang (gcc E only, `-Wtype-limits`): the constant is inside the type's range but the comparison is always true/false: `uc > 255`, `uc >= 0`, `uc < 0`, `uc <= 255`, `sc >= -128`, `sc <= 127`,
`sc < -128`, `x > 2147483647LL`, `x <= 2147483647LL`, `x >= -2147483648LL`, `u < 0`, `u >= 0`, and the bitwise forms `(uc & vc) == 300`, `(uc | vc) > 255`, `(uc ^ vc) == 256`
(promoted to int, 300 is in range). These are the "-Wtype-limits shapes stay accepted" of the plan.
`u == -1`: -Wsign-compare, gcc C++ at -Wall (A), clang C/C++ and gcc C only at -Wextra: so under the union rule it is warned by one compiler in one mode at -Wall; it is a sign-compare shape, not an out-of-range one.
Enumerations: `e == 7` (enum with range 0..3) warns in clang **C++ only** (default); `e < 0` and `e > 3` are silent (gcc silent everywhere).
Near misses silent everywhere: `uc == 255`, `uc == 0`, `uc > 254`, `sc > 126`, `sc == -128`, `us < 65535`, `x < 2147483647LL`, `x > -2147483648LL`, `uc + 1 == 300`, `uc + 1 > 255` (int after promotion), `uc == vc`, `x == 1`, `e == 3`.
EDG: #186-D on `uc >= 0`, `uc < 0`, `u < 0`, `u >= 0` (unsigned compared with zero) and #514-D on `uc == -1` (both modes), no tag otherwise.

#### R11 overlapping comparisons (clang only: -Wtautological-overlap-compare, -Wall; gcc silent)
Warns (A, both modes, signed and unsigned, as value and condition): `x > 5 && x < 3`, `x < 3 && x > 5`, `x == 1 || x != 1`, `x != 1 || x == 1`, `x > 5 || x < 6`, `x < 6 || x > 5`,
`x == 1 && x == 2`, `x != 1 || x != 2`, `x > 5 && x < 3 && z`, `(x > 5) && (x < 3)`, `x > 5 && 3 > x`, `5 < x && x < 3`, `x >= 5 && x < 5`, `x > 5 && x <= 5`,
`x == 1 && x != 1`, `x <= 5 || x >= 5`, and `u > 5 && u < 3`, `u == 1 || u != 1`, `u > 5 || u < 6`, `u == 1 && u == 2`.
Surprise: `z && x > 5 && x < 3` is silent (`&&` is left-associative, so the last two comparisons are not the operands of one node; clang compares only the two operands of one `&&`/`||`).
Silent: `x >= 5 && x <= 5`, `x > 5 && x < 10`, `x > 5 && y < 3`, `x == 1 || x == 2`, `x < 3 || x > 5`, `x > 5 || x > 3`, `x > 5 && x > 3`.
Contracts are not seen by compilers, but the same shapes occur there (below).

#### R12 dangling else (-Wdangling-else: clang default, gcc -Wall; both modes)
Warns: `if (a) if (b) x = 1; else x = 2;`, with a loop in between (`if (a) while (b) if (c) ... else ...`, `for`), three nested ifs, followed by another statement.
Silent: braces around the inner if or around the then-part (`if (a) { if (b) x=1; else x=2; }`, `if (a) { if (b) x=1; } else x=2;`), `if (a) if (b) x=1; else x=2; else x=3;`
(both elses taken), `else if` chains, `while (a) if (b) ... else ...` (the outer is a loop), `if (a) if (b) x = 1;` (no else), `if (a) switch (b) { default: if (c) ... else ... }`,
and `if (a) do if (b) ... else ... while (c);` (silent in all four).
EDG: nothing. (Loops in the probe files draw cerastes' `missing_decreases`, irrelevant to the compilers.)

#### R13 empty body (-Wempty-body)
clang by default, both modes, only when the `;` is on the SAME line as the controlling construct: `if (x > 0);`, `if (x > 0) ;`, `if (x) /* nothing */ ;`, `while (x--);`, `for (i = 0; i < n; i++);`, `switch (x);`.
gcc: `-Wextra` only (E) and only for if/else/do: `if (x > 0);`, `if (x > 0)\n ;`, `if (x) x = 1; else ;`, `do ; while (x--);`, the `/* nothing */` form; never for `while`, `for`, `switch`.
Silent in both: the `;` on the next line for clang (`if (x > 0)\n ;`, `while (x--)\n ;`, `for (...)\n ;`: gcc E only for the `if`), `if (x) ; else x = 1;` (all four silent), `if (x) {}`, `do {} while (x--);`, `while (x--) {}`, `for (...) {}`, `if (x) (void)0;`, `if (x) x = 1; else {}`, `{ ; }`.
By the union rule (default or -Wall): reject the same-line `;` body of `if`, `while`, `for`, `switch`; the next-line forms, `else ;`, `do ;` and `if (x) ; else` stay accepted (gcc -Wextra only or silent).

#### Existing tests and doc examples (clang run over all 1959 files, both modes; contracts scanned separately)
No file other than the new reject_warn_* ones is hit by R9, R11-bodies, R12 or R13 in a function body or any construct the compilers see.
Hits that a new rule could turn into a new or changed diagnostic:
* R10, enum range (clang C++ only, `-Wtautological-constant-out-of-range-compare`): `tests/scalar/negative/reject_enum_eq_negative_const.c`, `reject_enum_eq_negative_literal.c`, `reject_enum_eq_other_negative_enumerator.c`, `reject_enum_eq_other_negative_enumerator_swapped.c`, `reject_enum_order_neg.c` (all `rejected unsupported_expression @ L11/12` today; a new enum-range rule would add a second diagnostic at the same line unless it is skipped when the comparison is already rejected).
* R11 in CONTRACTS (the compilers never see these): `tests/scalar/negative/logic_and_post.c:8` (`ensures r == (a > 0 || a < 10)`, a tautology on purpose), `tests/scalar/negative/static_assert_block_excluded.c:8` (`requires x >= 0 && x < 0`) and `tests/scalar/negative/switch_case_requires_false_shift.c:8` (`requires x > 0 && x < 0`), both deliberately contradictory preconditions. If R11 applies in annotations, these three tests flip (their expect lines would gain a rejection).
* -Wtautological-type-limit-compare (clang, not default, not -Wall; stays accepted): `tests/scalar/negative/uninit_loop_condition.c:14`; in contracts many `x <= 2147483647` (type-limits shapes; accepted).
* My contract scan types every identifier as `int`, so its `-Wtautological-constant-out-of-range-compare` hits on long/long long contracts (llong_arith.c, long_arith.c, bit_long_post.c, bit_int128*_post.c, ...) are artifacts; no real R10 hit in a contract was found, but R10 on contracts with narrow-type parameters was not exhaustively checked (a grep for narrow-type parameters would be needed once the rule exists).

## M4.4d: exact sets for preconditions and commas; constant masks as remainders

Record 0026. Measurements on the M4.4d binary in the worktree `.claude/worktrees/m4.2b1`, branch `m4.4d-exact-sets`;
evidence under `build/m4.4d/` (not committed). The baseline is local `main` `fce46de` (M4.4c + S4): `run_tests.py -j 8`
5388 passed, 0 failed, 1 skipped; 1822 `.vpr` in `build/m4.4d-baseline-vpr`; the VIR self-test 686 lowering cases,
203 rejection cases, 0 problems, in `build/m4.4d-baseline-vir`.

### M1: the full suite with exact sets (part A only) **PASS after one owner decision**

The prototype (dual sets, two steps) with a temporary trace (`CERASTES_M44D_TRACE`: one line per full expression that
falls back, or whose two families differ), run over the baseline's 2027 test files (`build/m4.4d/m1/`).
- **Verdicts:** 5386 passed, 2 failed: `tests/scalar/negative/reject_unsequenced_split_requires.c` in both modes now
  reports `precondition @ L29` instead of `unsequenced_calls @ L29` (the pre-approved retirement; `take(f(x), h(y))`
  with one `requires` per argument is accepted, and with no caller precondition both clauses may fail). No other
  verdict or error line changed; no exit 3.
- **Diff 1** (`diff -rq -x MANIFEST`): 12 `.vpr` changed (six tests, both modes), 1810 identical, 2 new (the retired
  test now reaches Silicon). π changed in `call_comma_sum.c` and its two twins (`(one(x), 1) + two(y)`: the sum's range
  check now has {one, two}; emitted one, two, check instead of two, check, one); an item moved after a comma's left
  operand in `uninit_guarded_comma.c` and its two twins (the write `v_x := 2` follows the left operand's calls). No old
  test has a precondition that moves (no `f(a, g(b))` shape with a `requires` on `a` alone), and no explicit
  termination check is printed or dropped.
- **Step 2 is today's Viper, from the trace:** every changed `.vpr` comes from a file with an `exact-differs` line, and
  every file without one is byte-identical; no file with such a line is identical.
- **The fallback population:** no existing test is accepted by step 2 (0 `fallback-accepted`); 27 `fallback-rejected`
  full expressions in 14 files, all `reject_*unsequenced*` tests (and the doc example `unsequenced_calls.c`), each still
  rejected with today's message.
- **Invariants:** no full expression of the 2027 files or of the 828 VIR cases has a base half outside its exact half,
  and PRE(c) never differs between the halves.
- **VIR self-test:** 6 of the 686 old cases changed text (`calls_comma_in_a_sum`, `_wrong`,
  `fx_comma_floor_does_not_leak`, and the three `uninit_write_after_guarded_read_comma*`), all reorderings; Silicon on
  them and on the 20 new `exact_*` cases: 26 passed. **One old case changed outcome:** `hook_drop_flag_access`
  (0025 item 1's guard) no longer fires, because its program now takes step 1, where the comma's value already orders
  the write after the flag check. Owner (2026-10-27): a self-test hook `force_base_sets` makes every full expression take
  step 2; the case sets it and keeps its program and message, and a hook-off twin shows step 1 orders the program
  without the guard.

### The force-base run: step 2 is the previous version's Viper **PASS**

With the self-test hook `force_base_sets` (every full expression takes step 2) and the mask rule built in, every
baseline program in both modes was emitted with `--cerastes_verify_selftest=force_base_sets --keep-vpr --timeout=1`
(the `.vpr` is written before Silicon starts; `build/m4.4d/m2/fb-*.txt`): 1780 of 1822 are byte-identical to
`build/m4.4d-baseline-vpr`; the 42 that differ (21 programs, both modes) all call `c_and_`/`c_uand_` in the baseline,
and after dropping the prelude's `and` functions, the mask lines and the lines with ` % <digit>` on both sides, all 42
are identical too. So the base projection reproduces the previous version's Viper outside the masks, measured on the
whole suite (audit round 1 item 8).

### M2: the mask rule (part B) **PASS**

`run_tests.py -j 8` over the 32 baseline programs whose `.vpr` calls `c_and_`/`c_uand_`, the new `tests/scalar/mask_*`
tests and twins, and the doc example `integer-semantics/mask_remainder.c` (68 sources): 240 passed, 0 failed. Diff 2:
42 changed `.vpr` (the same 21 programs as above), every one calling `c_and_`/`c_uand_` in the baseline; the 22 others
that call those functions are identical (their masks are not constants). `lint_viper.py` (with the new remainder rule):
136 files clean. The 42 files shrink from 895,409 to 787,075 bytes (the prelude loses its `and` functions); no test
nests complement masks on one operand (item 16 of audit round 1: the largest count of `%` in a body is 4, the prelude's
own `c_rem`). The new part-A tests, pins and the doc example `verifying/comma_order.c`: 72 passed, 0 failed, every
`expect:` line as written before the run.

### S22: the mask battery **PASS** (`build/m4.4d/s22/`)

- **Silicon evaluation:** 1182 closed asserts `c_[u]and_N(a, M) == rewrite(a)` at N = 32, 64, 128, signed and
  unsigned, k ∈ {0, 1, 8, 16, N/2, N − 1, N} where allowed, both forms, at the bounds, ±1, 2^k − 1, 2^k, −2^k,
  ±(2^k ± 1) and random operands: all verify; a control with one wrong claim fails; a Python two's complement
  reference agrees on all 1182.
- **Viper's `%`:** `(-5) % 4 == 3`, `(-4) % 4 == 0`, `(-2147483648) % 256 == 0`, `(-5) - (-5) % 4 == -8` and the symbolic
  `x < 0 ==> 0 <= x % 4 && x % 4 < 4` verify; the control `(-5) % 4 == -1` fails.
- **Z3 4.16.0, QF_BV, every operand:** for each N, signedness, allowed k and form, `a` extended by 2 bits, the N-bit
  `bvand` with the mask equals `bvsmod(a, 2^k)` (or `a − bvsmod(a, 2^k)`): 902 queries, all unsat; a wrong control is
  sat. (A first `int2bv`/`bv2int` encoding timed out and was abandoned.)
- **Through the driver** (`s22/c/shapes.c`, both modes): `x & 254u`, `& 256u`, `& 257u` and `x & -3` keep
  `c_[u]and_N`; `& 0xFFFFFFFFu`, `& 255u`, `255u & x` become `% 4294967296`, `% 256`; `& 0xFFFFFF00u`, `& ~15u`, `& -16`
  become `x - x % 256`, `x - x % 16`; `v & ~0u` with a 64-bit `v` is `v % 4294967296`; `long long x & ~0xFFu` keeps
  `c_and_64(x, 4294967295 - 255)` (the constant is positive in 64 bits), and `& ~0xFFLL` is `x - x % 256`.

### The `bit_mask.c [c23]` hang **GONE** (`build/m4.4d/hang/`)

Silicon run directly on the kept `.vpr` of `tests/scalar/bit_mask.c` and `tests/scalar/negative/bit_mask_post.c` in
c23 (neither calls a `c_[u]and_N` function now): with `--numberOfParallelVerifiers 1`, 10 runs each, every run passes
(the twin reports its one expected error), 1.5–1.6 s; with `--proverRandomizeSeeds` as well, 16 runs each, the same,
1.4–1.6 s. 0 hangs in 52 runs; before the rule, the one-verifier runs hung 10 of 10 (handoff §3, "From S4").

### After audit round 2: the complement form `2^k * (a \ 2^k)` **PASS**

Audit round 2 found that the first complement form `a - a % 2^k` doubles the emitted text at each level of nested
complement masks (284 KB at depth 14). The form is now `2^k * (a \ 2^k)` (`low::div_pow2`); the low-mask form
`a % 2^k` is unchanged. The facts above for S22, M2 and the hang were measured on the first form; for the second
(`build/m4.4d/`, rebuilt after `check.sh` #2):
- **Only the complement lines changed:** the 42 mask files re-emitted under `force_base_sets` (`fb2/`) differ from the
  first form's in 34 lines, every one exactly `(X - (X % P))` → `(P * (X \ P))` (`transform_cmp.py`: 0 unexplained);
  over the VIR cases, 59 files and 65 lines, all the same transform.
- **S22 for the new form** (`s22b/`): 591 Silicon asserts `c_[u]and_N(a, M) == 2^k * (a \ 2^k)` at N = 32, 64, 128,
  signed and unsigned, every allowed k in {0, 1, 8, 16, N/2, N − 1, N}, negative operands and the bounds: all verify;
  the control fails; a Python reference agrees on all 591. The identity `a − a mod 2^k = 2^k · (a div 2^k)` was proved
  with Z3 4.16.0 for every k from 0 to 128 (no file kept; the auditor reran k ∈ {0, 1, 2, 63, 64, 127, 128}: unsat).
  Viper's `\` is SMT-LIB's `div` (`ExpressionTranslator.scala:149`, `TermToSMTLib2Converter.scala:205`), floor
  division for a positive divisor (`(div -5 16) = -1`).
- **Verdicts:** the 68 mask sources (32 baseline programs, the new tests and twins, the doc example): 240 passed, 0
  failed; the 124 VIR mask `.vpr` (`vir3-mask/`): 124 passed.
- **The hang** (`hang2/`): `bit_mask.c` and `bit_mask_post.c` [c23] with `--numberOfParallelVerifiers 1`, 10 runs each:
  every run gives its expected outcome in about 1.6 s.
- **Nesting** (`nest/`): 14 nested complement masks emit about 1 KB (976–1078 bytes), linear in the depth.
- **Lint:** `lint_viper.py`, extended to `\`, is clean on every `.vpr` above.
