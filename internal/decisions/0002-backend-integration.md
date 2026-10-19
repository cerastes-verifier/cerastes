# 0002 — Back-end integration with EDG

- **Status:** accepted (confirmed by the M1 spike and three soundness-audit passes)
- **Milestone:** M1 (2026-10-13)

## Context

cerastes has to read EDG's IL after the front end has finished: the routines,
the prototype instantiations of templates (their generic bodies), the instance
routines and their template arguments, and the `[[cerastes::…]]` attributes.
plan.md (M1) asks for one of two integrations: an in-process back end, modeled
on how the owner's CIR generator hooks in, or writing an IL file and reading it
with `il_read`. The default is in-process.

Constraints:

- **Invariant 5.** `third_party/edg` pins an upstream `edgcpp/compiler` commit
  (`9847356d`, decision 0001), so cerastes cannot rely on any EDG source change
  that is not upstream.
- **No front-end library.** At the pin, EDG's CMake compiles its sources
  straight into `cpfe`, `cpfe-cp` and `cdisp` (`src/cmake/*/CMakeLists.txt`).
  There is no library target and no install or export. The macro-configuration
  root is `${CMAKE_SOURCE_DIR}/cmake/macro-conf` (`cmake/macro-conf.cmake:67`),
  and about 16 uses of `CMAKE_SOURCE_DIR` stop EDG's CMake from working as an
  `add_subdirectory` of another project.
- **Neither stock binary has the IL M1 needs.**
  - `cpfe` lowers the IL to C, and lowering drops the prototype instantiations.
    With `-N` / `--no_il_lowering` it also suppresses the back end
    (`cmd_line.c:10657-10665`).
  - `cpfe-cp` keeps unlowered IL but has `ALL_TEMPLATE_INFO_IN_IL=0`
    (`--dump_configuration`).
- **The owner's CIR integration** (fork branch `cir`, not upstream) adds a
  `BACK_END_TYPE` macro (`src/back_end_type.h`) and a streaming ECIL writer
  inside EDG. Both are EDG source changes. Its `BACK_END_TYPE_EXTERNAL`
  ("a user-supplied `back_end()`") formalizes a hook that upstream already has
  in legacy form.

The upstream hook (`src/cfe.c:50-57`, call at `:170`):

```c
#if BACK_END_SHOULD_BE_CALLED && \
    !BACK_END_IS_C_GEN_BE && !BACK_END_IS_CP_GEN_BE
extern void back_end(void);
#endif
```

`cfe_main` calls `back_end()` after `fe_wrapup()`. It skips the call when there
were errors or when `--no_code_gen` sets `suppress_back_end`.
`doc/source/build.rst:328-356` documents this: with both `BACK_END_IS_*`
macros FALSE, "some other back end must be supplied".

## Decision

**In-process back end. cerastes compiles EDG's front-end sources itself, with
the cerastes configuration, and links its own `back_end()`.** No EDG source
file changes.

1. **Build.** A top-level `CMakeLists.txt` in this repository builds:
   - a static library `edg_fe` from the `third_party/edg/src` files that make
     up `cpfe` (`CORE_FRONT_END_SOURCE_FILES` and the `add_src_file_if_present`
     additions in `src/CMakeLists.txt:206-234`). It uses EDG's own compile
     flags for cpfe from `build/edg/build.ninja` (`-std=c++14 -pedantic -Werror=shadow -Wconversion … -x c++`).
   - an INTERFACE target `edg_config` with the configuration macros and EDG's
     include directories. Every target that includes EDG headers links it.
   - our back end (`src/backend/`) and a small `main` (`src/driver/`).

   The build directory is `build/cmake` and the executable is
   `build/cerastes-fe`. That leaves `build/cerastes` free for the M3 driver.
2. **Configuration.** The macros are passed as `-D` flags. They take precedence
   because every default in `src/host_envir.h` and every line of the generated
   `cmake_defines.h` is wrapped in `#ifndef`.

   | Macro | Value | Why |
   |---|---|---|
   | `BACK_END_IS_C_GEN_BE` | 0 | external back end (`cfe.c:50-57`) |
   | `BACK_END_IS_CP_GEN_BE` | 0 | external back end |
   | `DO_IL_LOWERING` | 0 | keep C++ IL; lowering drops prototype instantiations |
   | `IL_SHOULD_BE_WRITTEN_TO_FILE` | 0 | IL stays in memory. `fe_wrapup` then frees only the front-end region (`fe_wrapup.c:843-855`). |
   | `PROTOTYPE_INSTANTIATIONS_IN_IL` | 1 | M1 setting. It is forced TRUE anyway (`host_envir.h:769-775`), but setting it explicitly also implies `ALL_TEMPLATE_INFO_IN_IL` (`:745-755`). |
   | `ALL_TEMPLATE_INFO_IN_IL` | 1 | Generic bodies and template links in the IL. With no lowering it also makes `FUNCTION_PROTOTYPE_INSTANTIATION_DEFERRAL_ALLOWED` FALSE (`:787-801`), so templates that are never instantiated still get a prototype body. |
   | `RECORD_UNRECOGNIZED_ATTRIBUTES` | 1 | Keep `[[cerastes::…]]` as `ak_unrecognized` with raw tokens. The default follows `BACK_END_IS_CP_GEN_BE` (`:1951-1957`). |
   | `EXTRA_SOURCE_POSITIONS_IN_IL` | 1 | End positions and ranges. Already 1 in the stock debug configuration; stated explicitly. |
   | `INSTANTIATE_TEMPLATES_EVERYWHERE_USED` | 1 | See below |
   | `DEFAULT_INSTANTIATION_MODE` | `tim_used` | See below. The stock-generated `cmake_defines.h` hard-codes `tim_none`, so the derived default in `host_envir.h` never applies. |
   | `DEFAULT_REMOVE_UNNEEDED_ENTITIES` | 0 | The derived default for this back end is TRUE (`host_envir.h:1138-1147`). That would prune unused entities from the IL, including never-instantiated templates and unused functions, which a verifier must still check. The stock `cmake_defines.h` happens to say 0; we pin it. `build/cerastes-fe` also rejects `--remove_unneeded_entities`. |
   | `MAINTAIN_NEEDED_FLAGS` | 1 | IL-layout macro, inherited from the stock configuration; pinned so that a change to the stock base cannot change layouts behind our back |
   | `SAME_REPR_INTS_INTERCHANGEABLE_IN_IL` | 0 | Integer types of the same representation (e.g. `long` and `long long`) stay distinct in the IL; M3's per-type range checks need the declared type. Inherited, now pinned. |
   | `EDG_MAIN` | `edg_main` | Makes the front end callable (`basics.h:232-237`). Our `main` handles our options and calls `edg_main`. |
   | `USE_CMAKE_DEFINES` | 1 | As in EDG's own build: include the generated `cmake_defines.h` |
   | `FULLY_RESOLVED_MACRO_POSITIONS` | 1 | *M2 amendment.* Diagnostics for text from a macro argument point at the argument's own line and column, not at the start of the invocation (invariant 7; contracts are written through `CERASTES_*` macros, decision 0008). It adds `orig_seq` and `orig_column` to `a_source_position`, so it is layout-affecting and must reach every TU. The probe (M2 Phase 1) showed identical stdout, stderr and exit status on all 42 existing test files. |
   | `RECORD_MACROS_IN_IL` | 1 | *M3 amendment (re-look R4c, 2026-10-16).* EDG records every active `#define` and `#undef` in `il_header.macros`, with command-line ones flagged. The back end uses it to reject changes of the names the annotations depend on (`annotation_macro_changed`, decision 0008 item 2), which a source-text scan cannot do reliably (string literals, comments, inactive groups, `_Pragma`, macros built by token pasting). It adds IL entries (`iek_macro`), which the walker oracle ignores; no golden changed. It is pinned with the rest and checked by `scripts/check_edg_config.sh`. |
   | `INCLUDE_EDG_TEST_ATTRIBUTES` | 0 | *M2 amendment.* The debug base configuration turns on EDG's own test attributes. With attribute registration (0007) they would include a closed, registered `edg` namespace, whose attributes must never reach the cerastes back end. The setting also changes `an_attribute_kind`'s values, so it is pinned with the rest. |

   **`INSTANTIATE_TEMPLATES_EVERYWHERE_USED`.** Turning off both EDG back ends
   also turns on automatic template instantiation (`host_envir.h:960-971`,
   which is otherwise off only for the C++-generating back end). The default
   instantiation mode then becomes `tim_none` (`:912-918`).
   - Automatic instantiation hands instantiation to a prelinker and writes a
     `<source>.ti` file into the working directory.
   - `tim_none` means used templates do not get instance bodies in the
     translation unit.

   A verifier wants what an ordinary compiler does: every used template
   instantiated in the translation unit. `INSTANTIATE_TEMPLATES_EVERYWHERE_USED=1`
   makes `AUTOMATIC_TEMPLATE_INSTANTIATION` FALSE. That holds in C mode too,
   so no command-line option is needed. It requires
   `LINKER_CAN_DISCARD_DUPLICATE_DEFINITIONS`, which is 1 in the stock
   configuration.

   It should also make the default mode `tim_used`. But the stock
   `cmake_defines.h` defines `DEFAULT_INSTANTIATION_MODE tim_none` outright,
   so we also pass `DEFAULT_INSTANTIATION_MODE=tim_used`. Both are confirmed
   with `--dump_configuration`, and used templates now have instance routines
   with bodies. The M1 spike found this setting: the first build wrote `.ti`
   files.

   `GENERATE_SOURCE_SEQUENCE_LISTS` is left at its default (FALSE for this
   back end). The walker uses scope lists, not source sequence lists.

   **Guards.** `src/backend/edg_il.h` has a `static_assert` for every required
   macro value, so a TU compiled without `edg_config` does not build.
   `scripts/check_edg_config.sh` (run by `check.sh`) checks the values in
   `build/cerastes-fe --dump_configuration`. Together they catch drift in our
   `-D` flags and in the inherited `cmake_defines.h`.
3. **Generated headers.** `cmake_defines.h`, `err_codes.h` and `err_data.h` are
   taken from the stock EDG build (`build/edg/src/includes`), which
   `scripts/bootstrap.sh` already runs. That build uses the same pin and the
   same base macro configuration (`macos-arm-clang`). Our `-D` flags override
   its back-end-specific settings.
4. **Layout safety.** The IL is a set of C structs whose layout depends on
   about 160 `#if`s on `BACK_END_IS_*` and `DO_IL_LOWERING` in `il_def.h`. A
   back-end translation unit compiled with different macros would read the IL
   silently wrong. So the macros live in exactly one place, `edg_config`. Every
   TU that includes an EDG header gets them from there, and nothing else
   defines them.

   In this configuration `USE_EDG_NAMESPACE` is 0, so EDG's symbols
   (`il_header`, `a_routine_ptr`, `edg_main`, …) are global. A TU that includes
   EDG headers must include `basics.h` first, because it defines
   `BEGIN_EDG_NAMESPACE` and the configuration. EDG's sources are compiled as
   exactly the flags of stock cpfe (`-std=c++14 -g3 -Wall -Wwrite-strings
   -Wformat-security -pedantic -Werror=shadow -Wimplicit-fallthrough
   -Wconversion -Wsign-conversion -fno-exceptions -fno-unwind-tables -fno-rtti
   -x c++`; only the defines differ), and our sources as C++20 (the EDG
   headers compile cleanly as C++20). The M1 audit checked that the IL
   struct layouts our walker uses (`sizeof`/`offsetof` of 24 structs and 30
   fields) are identical under C++14 and C++20 with these macros. EDG's
   optional softfloat library is linked only when
   `EDG_SOFTFLOAT_LIB_BASE_PATH` is set, and it is not needed here.
5. *(Amended in M2: once EDG can register attribute namespaces (0007), cerastes
   registers `cerastes` as a closed namespace, and the M1 probes that used
   made-up `cerastes::` names were moved to the unregistered namespace
   `unreg_ns`, byte-identical apart from the name. The text below is the M1
   state.)*
   **Warning #2803.** EDG always warns "attribute namespace "cerastes" is
   unrecognized", even when unrecognized attributes are recorded
   (`attribute.c:1108-1121`; known namespaces are hard-coded at `:970-980`).
   cerastes does not suppress it globally: a misspelled namespace such as
   `cerastse::` must stay visible. Tests that use `cerastes::` attributes
   before M2 record the warning in their expected output. M2 registers the
   namespace, which removes the warning for `cerastes::` only.

## Alternatives considered

- **IL file plus `il_read` in a separate program** (as `cdisp` does). This
  works with stock `cpfe -N -o x.cil`, but:
  - The file is a raw dump of the in-memory structs (`il_write.c:433`).
  - The only compatibility check is a version string, so a reader built with
    different macros misreads the file silently.
  - Stock `cpfe` lowers and has no full template information.

  We would still have to build EDG with our own configuration. Nothing is
  gained over in-process, and we would add a file format to keep in sync.
- **The CIR-style `BACK_END_TYPE` hook and in-EDG writer.** Not upstream, so it
  would break invariant 5. If `BACK_END_TYPE` lands upstream, our configuration
  becomes `BACK_END_TYPE=BACK_END_TYPE_EXTERNAL`, which is the same hook.
- **EDG's own CMake with a cerastes macro-conf** (reachable from outside the
  tree through a `../` path in `EDG_MACRO_CONF`). This can build a
  `cpfe`-like binary with our macros, but it cannot link our code. Also, a
  missing `.cmakedef` would be auto-written into the submodule.

## Consequences

- cerastes builds EDG twice: the stock build (`build/edg`, still used by
  `scripts/cpfe.sh` and the `edg-smoke` tests) and our configuration
  (`build/cmake`). The extra build takes seconds.
- `build/cerastes-fe` accepts every `cpfe` option, including `--il_display`
  and `--dump_configuration`, so IL evidence comes from the exact
  configuration cerastes uses.
- If EDG adds or renames source files, `cmake/EdgFrontEnd.cmake` must follow.
  Moving the pin re-runs `check.sh`, whose golden tests then fail.
- The dependence on `build/edg/src/includes` ties our build to the stock
  build's base configuration. If that becomes a problem, generate the headers
  with EDG's `dev_tools/bin/edg-cmakedef` and `mk_errinfo` instead.
