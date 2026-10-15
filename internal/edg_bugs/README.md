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
  one discrete commit that fixes the bug, with its test. The owner pushes
  branches and files the issues: sessions never open upstream issues, PRs or
  Discussions (decision 0003 item 6).
- **Reproductions** were run on upstream `main` `034385ba0a`, built natively
  with the `macos-arm-clang-debug` preset and run as `HACKING.md` describes
  (`--clang_version=210000`, the scraped system include directories). Apple
  clang 21 (Command Line Tools 27.0, SDK 27.0) is the reference. Each file
  contains its whole reproducer and the outputs quoted from those runs.

## Index

| No. | Bug | Status |
|---|---|---|
| [0001](upstreamed/0001-attribute-namespace-prefix-match.md) | An attribute namespace that is only a prefix of a known one is accepted | Upstreamed |
| [0002](upstreamed/0002-test-filter-non-file-scope-pointer.md) | The test output filter does not normalize `**possible non-file-scope ptr** (0x…)` | Upstreamed |
| [0003](0003-attribute-parameter-pack-substituted-empty.md) | A function parameter pack in an attribute argument is substituted as an empty pack | open |
| [0004](0004-attribute-parameter-after-pack-wrong-parameter.md) | A parameter after a function parameter pack, named in an attribute argument, is the wrong parameter in an instance | open |
| [0005](0005-macos-base-predefines-blocks.md) | The macOS clang base predefines `__BLOCKS__`, but the front end cannot parse block syntax | open |
| [0006](0006-macos-base-missing-interference-size-macros.md) | The macOS clang base lacks `__GCC_DESTRUCTIVE_SIZE` and `__GCC_CONSTRUCTIVE_SIZE` | open |
| [0007](0007-clang-ext-vector-type-unsupported.md) | Native macOS builds do not support `ext_vector_type` (or `vector_size`), which libc++ uses | open |

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
