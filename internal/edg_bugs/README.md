# EDG bugs

Bugs in upstream EDG (`edgcpp/compiler`) found while building cerastes. One
file per bug, numbered like the decision records: `NNNN-short-name.md`.

- **Format.** Each file follows the shape of
  [edgcpp/compiler#252](https://github.com/edgcpp/compiler/issues/252), so the
  owner can file it as it is:
  - title;
  - labels;
  - `## Summary`;
  - `## Reproducer`, with **Actual:** and **Expected:**;
  - `## Confirmation`;
  - `## Suggested fix` (or `## Fix` when it is fixed);
  - `## Impact on cerastes`.

  A header block says where the bug was found and its status.
- **What counts.** A bug in upstream EDG, reproduced on an unmodified upstream
  build. A bug in the cerastes fork series is fixed in the series and does not
  get a file here.
- **Fixed bugs get a branch** in the fork clone `~/Documents/compiler`, named
  with the bug's number (`0001`). It is based on upstream `main` and holds the
  one discrete commit that fixes the bug, with its test. When one fix builds
  on another, its branch is stacked on the other's branch and holds one
  commit on top of it; the bug file says so.
- **Fixed bugs are also merged** into the fork's `cerastes` branch (with
  `--no-ff`, so the branch's commit is kept as it is), and cerastes moves its
  pin to the merge. The owner pushes
  branches and files the issues: sessions never open upstream issues, PRs or
  Discussions (decision 0003 item 6).
- **Reproductions** were run on upstream `main` (`034385ba0a` for 0001–0007,
  `d6f83174c7` from 0008 on and for the fixes up to 0016; no file under `src/`,
  `bases/` or `cmake/` differs between the two; `307002eae6` from 0020 on,
  which changes `src/decl_inits.c` and `src/cp_gen_be.c`; `ec2f12cffa` for 0022), built natively
  with the `macos-arm-clang-debug` preset and run as `HACKING.md` describes
  (`--clang_version=210000`, the scraped system include directories). Apple
  clang 21 (Command Line Tools 27.0, SDK 27.0) is the reference. Each file
  contains its whole reproducer and the outputs quoted from those runs.

## Index

| No. | Bug | Status |
|---|---|---|
| [0001](upstreamed/0001-attribute-namespace-prefix-match.md) | An attribute namespace that is only a prefix of a known one is accepted | Upstreamed |
| [0002](upstreamed/0002-test-filter-non-file-scope-pointer.md) | The test output filter does not normalize `**possible non-file-scope ptr** (0x…)` | Upstreamed |
| [0003](0003-attribute-parameter-pack-substituted-empty.md) | A function parameter pack in an attribute argument is substituted as an empty pack | fixed on branch `0003` (`enable_if`; stacked on `0004`); not filed |
| [0004](0004-attribute-parameter-after-pack-wrong-parameter.md) | A parameter after a function parameter pack, named in an attribute argument, is the wrong parameter in an instance | fixed on branch `0004` (`enable_if`); not filed |
| [0005](upstreamed/0005-macos-base-predefines-blocks.md) | The macOS clang base predefines `__BLOCKS__`, but the front end cannot parse block syntax | Upstreamed |
| [0006](upstreamed/0006-macos-base-missing-interference-size-macros.md) | The macOS clang base lacks `__GCC_DESTRUCTIVE_SIZE` and `__GCC_CONSTRUCTIVE_SIZE` | Upstreamed |
| [0007](0007-clang-ext-vector-type-unsupported.md) | Native macOS builds do not support `ext_vector_type` (or `vector_size`), which libc++ uses | fixed on branch `0007`; not filed |
| [0008](0008-enable-if-address-of-release-local-constant.md) | Taking the address of a function with an `enable_if` whose condition is a constant is an internal error | open |
| [0009](0009-enable-if-default-argument-not-evaluated.md) | A call that omits a defaulted parameter named in an `enable_if` condition is rejected | open |
| [0010](0010-enable-if-class-pack-elements-indistinguishable.md) | Elements of a class template parameter pack named in the `enable_if` of a member function are indistinguishable | open |
| [0011](0011-c-gen-be-vector-fill-needs-gcc-target.md) | The C-generating back end asserts on a vector operation when the generated-code target is not GCC | open |
| [0012](0012-builtin-shufflevector-error-operand-assertion.md) | `__builtin_shufflevector` with an operand that becomes an error operand is an internal error | open |
| [0013](0013-ext-vector-type-element-access-unsupported.md) | Element access on an `ext_vector_type` vector (`v.x`, swizzles) is unsupported | open |
| [0014](0014-ext-vector-size-deduction-requires-int.md) | The size of an `ext_vector_type` alias template is deduced only into an `int` parameter | open |
| [0015](0015-enable-if-reference-parameter-not-evaluated.md) | An `enable_if` condition that names a reference parameter rejects every call (and, after a pack, crashes) | open |
| [0016](0016-adjacent-function-parameter-packs-element-count.md) | Two adjacent function parameter packs: one pack's element count includes the other's | fixed on branch `0016`; not filed |
| [0017](0017-generic-lambda-in-function-template-declaration-crash.md) | A generic lambda with more `auto` parameters than the function template has template parameters, in its declaration, crashes the front end | open |
| [0018](0018-empty-class-pack-and-member-pack-in-one-expansion.md) | An empty class pack and a member template's pack in one expansion: "identifier is undefined" at the class instantiation | open |
| [0019](0019-cxx23-parameter-names-in-nested-function-type-trailing-return.md) | In C++23, parameter names of a nested function type are not found in its trailing return type in some contexts | open |
| [0020](upstreamed/0020-lambda-in-function-template-declaration-names-template-parameter.md) | A lambda that names a template parameter in a function template's declaration cannot be substituted: "identifier is undefined" | Upstreamed |
| [0021](0021-lambda-called-in-constant-template-argument-not-folded-std-mode.md) | A lambda called in a constant template argument of a function template is not folded in std mode: "template instantiation resulted in unexpected function type" | open |
| [0022](0022-suppressed-note-internal-error.md) | Suppressing the number of a constant-evaluation note is an internal error: "determine_severity_code: bad severity" | fixed on branch `0022` (on `ec2f12cffa`); not filed; not in the pin (box suites pending) |

0008 to 0019 were found while fixing 0003 to 0007 and 0016. They are
recorded only. On 2026-10-15 the owner decided that only bugs that block
cerastes are fixed. 0016 was fixed because 0003's folds over adjacent packs
need it.

0005, 0006 and 0007 are the three causes of the libc++ failures recorded
in decision 0001: `<utility>`, `<string>`, `<vector>`, `<algorithm>` and
`<iostream>` from SDK 27.0.

## Investigated, not filed

These candidates were recorded as possible EDG bugs during M1 and M2, but did
not reproduce on upstream (2026-10-15):

- **Class-template parameters left in the attributes of members** (upstream
  draft §11 issue 3, fixed for registered attributes by fork commit F). With
  `enable_if`, upstream substitutes the class's parameters correctly. It is
  shown only with attributes registered through the fork series.
- **Internal error for an expression attribute after the identifier list of
  an old-style definition** (upstream draft §11 issue 4). With `enable_if`,
  `diagnose_if` and unknown attributes, upstream reports `identifier "x" is
  undefined`, which is correct. The internal error was in the series'
  registered-attribute path.
- **`il_display` omits attributes attached to statements** (M1). Upstream's
  `cpfe-cp`, which records unrecognized attributes, displays them. The
  omission shows only in the cerastes configuration.
- **Clang and GNU modes drop misplaced attributes with only a warning**
  (upstream draft §11 issue 5). This is what clang and GCC do too: a design
  question for a Discussion, not a bug.
- **Crashes and assertions fixed by fork commits H, J and K.** They are in
  code the series added, so they are not upstream bugs.

Noticed but not investigated: upstream accepts `diagnose_if` and never
diagnoses, even on a plain function, where clang reports the error. It was
not found during the cerastes work, so it is not filed here.
