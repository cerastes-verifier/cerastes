# EDG IL inventory (M1)

**Status: complete. Gate G1: PASS (see "Gate G1 verdict"). Walker output is cross-checked against `il_display`, and the goldens are in `tests/il/`.**
All excerpts below come from `build/cerastes-fe` with
`DEFAULT_INSTANTIATION_MODE tim_used`, `AUTOMATIC_TEMPLATE_INSTANTIATION 0`,
`PARENS_IN_IL 0`, `PROTOTYPE_INSTANTIATIONS_IN_IL 1`,
`ALL_TEMPLATE_INFO_IN_IL 1`, `RECORD_UNRECOGNIZED_ATTRIBUTES 1`
(`--dump_configuration`).

## Purpose

Gate G1 asks whether EDG's prototype-instantiation IL carries what the generic
proof of M4 needs. VeriFast answers that question with Clang AST queries
(`GenericFunctionChecker` and friends in `src/cxx_frontend/ast_exporter/`). This
file maps every such query to the EDG IL field that answers it, with
`--il_display` evidence, and records the five G1 checks.

## Setup

- EDG pin: `third_party/edg` at `9847356d` (upstream `edgcpp/compiler`).
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

  This matters for M4's fail-closed rule: a cast to `T` of a literal is an
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

Checks 1 and 2 pass, so the fallback (decision 0003, a Clang front end) is not
triggered.

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
| F5 | :691 | `arg.getAsType().getCanonicalType()` | canonical argument type | `skip_typerefs(arg->variant.type)` | `check3` `inc<unsigned char>`: the `template_arg_list` entry prints `type: ...: unsigned char`. BUT `verifast-plain/integral.cpp` `count_to<uint16_t>`: the entry is `type: ...: uint16_t`, a type entry with `kind: tk_typeref`, `typeref_type: ...: unsigned short`; the golden prints `template count_to args <T=uint16_t>`. (By contrast, `twice<char32_t>`'s entry is a plain `tk_integer`, `int_kind: unsigned int`, `char32_t_type: TRUE`: whether a typeref appears depends on how the argument was spelled.) | Typerefs DO occur in `template_arg_list`. M4 must skip typerefs (`skip_typerefs`) before deciding that an argument is a builtin integer type; classifying the entry's own `kind` would reject `uint16_t` arguments. |
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
| G3 | :528-534 | `decl->isImplicit()` | implicit templates (deduction guides) have no contract | `source_corresp.is_generated`-style flag, or a compiler-generated template; not found at this reading | not probed: UNCLEAR (no implicit templates in the probes). | Gap: confirm in `il_def.h` whether `a_template` marks implicit templates; M4 only cares about function templates, so low risk. |
| G4 | :538-544 | `getDefinition()`, `getDescribedFunctionTemplate()`, `owner != decl` | serialize specializations once per redeclaration set | `definition_template` / `canonical_template` (E4, E6): only act on the canonical entry | as E4. | |
| G5 | :547 | `decl->specializations()` | all instances | no list: scan routines (E7) | as E7. | |
| G6 | :549-553 | `isExplicitInstantiationOrSpecialization()` | reject explicit instantiation and specialization | `routine->is_specialized`, `explicit_instantiation`, `specialized_with_old_syntax` (`il_def.h:12533`, `12565`, `12542`) | as F2. | Covered by `specializations.cpp` (see F2). |
| G7 | :554 | `usesGenericProof(spec)` | split instances | F1-F7 | as F1-F7. | |
| G8 | :566 | `spec->getSourceRange()` | position of an instance | `routine->source_corresp.decl_position` (`il_def.h:3327`), `a_template.definition_range` (`il_def.h:17359`) | `generic_body.cpp`: routine `decl_position.seq: 40` (the declarator of the definition), template entries `decl_position.seq: 34` and `39`. `EXTRA_SOURCE_POSITIONS_IN_IL` ranges (`expr_range`, `identifier_range`, `declarator_range`) are printed for every entry. | `EXTRA_SOURCE_POSITIONS_IN_IL` provides ranges. |
| G9 | :567 | `serializeFunctionDecl(spec)` | the instance body | instance routine body (conventions above); instance routines are real IL (not prototype) | `check3`: every instance has `function_def_number` (a body) because `tim_used` instantiates what is used. An unused template has no instance routine at all (`never_used` in `check2` has only its prototype). | An instance without a body (declared only, not required) has `function_def_number == NULL_function_def_number`. |

Total rows: 74 (A: 9, B: 8, C: 11, D: 15, E: 10, F: 12, G: 9 — see the tables; some are
shared-reference rows). Rows with no direct field (marked **(derived)**): A4,
A8, A9, B3, B4, C1, C2, C6, D1, D12, E7, E8, F2, F7.

## Gotchas for M2/M4

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
    `tk_typeref` to `unsigned short`; M4 must skip typerefs before classifying an
    argument (F5). `bool` is `int_kind: char` + `bool_type: TRUE`, so excluding it
    needs `bool_type` (B8).
