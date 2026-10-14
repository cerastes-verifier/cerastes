#!/usr/bin/env bash
# Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
# Idempotent macOS (Apple silicon) bootstrap: installs prerequisites, checks
# out the submodules, builds EDG (stock, for scripts/cpfe.sh, and the cerastes
# configuration, build/cerastes-fe; with CERASTES_EDG_SRC set, another EDG tree
# in build/override, decision 0005), builds Silicon, fetches the pinned Z3,
# installs the documentation tool chain (Node, `npm ci` in doc/) and writes
# build/env.sh.  Every pin is recorded in internal/decisions/0001-toolchain-pins.md;
# change a pin here and there together.
#
# Usage: scripts/bootstrap.sh [--build-only] [--force-silicon]
#   --build-only     skip Homebrew, the Z3 download and `npm ci`.  It still
#                    initializes missing submodules, rebuilds EDG, cerastes-fe
#                    and (when its stamp is stale) Silicon, where sbt may fetch
#                    dependencies, and rewrites build/env.sh.  Stale or missing
#                    documentation dependencies (doc/node_modules) are an
#                    error that points here.  Used by scripts/check.sh.
#   --force-silicon  rebuild silicon.jar even if the build stamp is current.

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD="$ROOT/build"
TOOLS="$BUILD/tools"

# ---- Pins -------------------------------------------------------------------
EDG_PRESET=macos-arm-clang-debug
JDK_FORMULA=openjdk@17          # sbt 1.6.2 (pinned by Silicon) fails on JDK 27
Z3_VERSION=4.16.0               # the version Silicon's CI tests against
Z3_ASSET=z3-4.16.0-arm64-osx-15.7.3
Z3_SHA256=41828fa07d5cb77bfaee326e8e6dac074f26329c09c633f9e66012bb917cf8ae
Z3_URL="https://github.com/Z3Prover/z3/releases/download/z3-$Z3_VERSION/$Z3_ASSET.zip"

DOC_NODE_VERSION_FILE="$ROOT/doc/.node-version"   # Node major; CI reads it too

BUILD_ONLY=0
FORCE_SILICON=0
for arg in "$@"; do
  case "$arg" in
    --build-only) BUILD_ONLY=1 ;;
    --force-silicon) FORCE_SILICON=1 ;;
    -h|--help) sed -n '5,19p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
    *) echo "bootstrap.sh: unknown option: $arg" >&2; exit 2 ;;
  esac
done

log() { printf '==> %s\n' "$*"; }
warn() { printf 'warning: %s\n' "$*" >&2; }
die() { printf 'bootstrap.sh: error: %s\n' "$*" >&2; exit 1; }

[[ "$(uname -s)" == Darwin ]] || die "only macOS is supported"
[[ "$(uname -m)" == arm64 ]] || die "only Apple silicon is supported (EDG preset $EDG_PRESET)"

mkdir -p "$BUILD" "$TOOLS"

# ---- 1. Homebrew prerequisites ----------------------------------------------
if [[ $BUILD_ONLY -eq 0 ]]; then
  command -v brew >/dev/null || die "Homebrew is required: https://brew.sh"
  xcode-select -p >/dev/null 2>&1 || die "Xcode Command Line Tools are required: xcode-select --install"
  # cmake, ninja, python and shellcheck are satisfied by any copy on PATH; sbt
  # and the JDK must be the Homebrew ones because env.sh points at them.
  for tool in cmake ninja python3 shellcheck; do
    if ! command -v "$tool" >/dev/null; then
      formula=$tool; [[ $tool == python3 ]] && formula=python
      log "brew install $formula"
      brew install "$formula"
    fi
  done
  # Node builds the documentation (doc/, Antora); any node and npm on PATH do.
  if ! command -v node >/dev/null || ! command -v npm >/dev/null; then
    log "brew install node"
    brew install node
  fi
  for formula in sbt "$JDK_FORMULA"; do
    if ! brew list --formula --versions "$formula" >/dev/null; then
      log "brew install $formula"
      brew install "$formula"
    fi
  done
fi
JAVA_HOME="$(brew --prefix "$JDK_FORMULA")/libexec/openjdk.jdk/Contents/Home"
[[ -x "$JAVA_HOME/bin/java" ]] || die "JDK not found at $JAVA_HOME (brew install $JDK_FORMULA)"
export JAVA_HOME

# ---- 2. Submodules ------------------------------------------------------------
# Initialize missing submodules, but never move one that is already checked
# out: a developer may have EDG work in progress there.
ensure_submodules() {
  local repo="$1" prefix="$2" line state path
  while IFS= read -r line; do
    [[ -z "$line" ]] && continue
    state="${line:0:1}"
    path="$(awk '{print $2}' <<<"${line:1}")"
    case "$state" in
      -) log "git submodule update --init $prefix$path"
         git -C "$repo" submodule update --init -- "$path" ;;
      +) warn "$prefix$path is not at the pinned commit (left as is; check.sh fails on it). Do not check fork commits out there: set CERASTES_EDG_SRC instead (decision 0005)" ;;
      U) die "$prefix$path has merge conflicts" ;;
    esac
    ensure_submodules "$repo/$path" "$prefix$path/"
  done < <(git -C "$repo" submodule status)
}
ensure_submodules "$ROOT" ""

# ---- 3. EDG ---------------------------------------------------------------------
# The EDG tree is third_party/edg (kind "upstream") or, when CERASTES_EDG_SRC is
# set, that checkout (kind "override"); decision 0005.  Each kind has its own
# build directories, so switching never rebuilds the other one.
if [[ -n "${CERASTES_EDG_SRC:-}" && "$CERASTES_EDG_SRC" != /* ]]; then
  echo "bootstrap.sh: CERASTES_EDG_SRC=$CERASTES_EDG_SRC is a relative path; use an absolute path (decision 0005)" >&2
  exit 2
fi
resolved="$(python3 "$ROOT/scripts/edg_tree.py" resolve)" \
  || die "CERASTES_EDG_SRC is not a usable EDG tree (see the message above)"
EDG_KIND="${resolved%% *}"
EDG_SRC="$(cd "${resolved#* }" && pwd -P)"
if [[ "$EDG_KIND" == override ]]; then
  OUT="$BUILD/override"
  warn "building EDG from CERASTES_EDG_SRC=$EDG_SRC (decision 0005); check.sh refuses this on branch main"
else
  OUT="$BUILD"
fi
EDG_BUILD="$OUT/edg"
CERASTES_BUILD="$OUT/cmake"
EDG_STAMP="$EDG_BUILD/edg-tree.stamp"
mkdir -p "$OUT"
rel() { printf '%s' "${1#"$ROOT"/}"; }

# A build of a different tree (the stamp, or failing that the CMake cache) is
# wiped; the same tree at another commit or with other edits is built
# incrementally.
cache_home() {
  [[ -f "$1/CMakeCache.txt" ]] || return 0
  sed -n 's/^CMAKE_HOME_DIRECTORY:INTERNAL=//p' "$1/CMakeCache.txt" | head -1
}
stamped_tree=""
[[ -f "$EDG_STAMP" ]] && stamped_tree="$(sed -n 's/^tree //p' "$EDG_STAMP" | head -1)"
cached_tree="$(cache_home "$EDG_BUILD")"
if { [[ -n "$stamped_tree" && "$stamped_tree" != "$EDG_SRC" ]]; } \
   || { [[ -n "$cached_tree" && "$(cd "$cached_tree" 2>/dev/null && pwd -P)" != "$EDG_SRC" ]]; }; then
  log "EDG tree changed; removing $(rel "$EDG_BUILD") and $(rel "$CERASTES_BUILD")"
  rm -rf "$EDG_BUILD" "$CERASTES_BUILD"
fi

if [[ ! -f "$EDG_BUILD/build.ninja" ]]; then
  log "cmake --preset $EDG_PRESET (out of tree: $(rel "$EDG_BUILD"))"
  cmake -S "$EDG_SRC" --preset "$EDG_PRESET" -B "$EDG_BUILD" > "$OUT/edg-configure.log" 2>&1 \
    || { tail -20 "$OUT/edg-configure.log"; die "EDG configure failed ($(rel "$OUT")/edg-configure.log)"; }
fi
log "ninja -C $(rel "$EDG_BUILD")"
ninja -C "$EDG_BUILD" > "$OUT/edg-build.log" 2>&1 \
  || { tail -30 "$OUT/edg-build.log"; die "EDG build failed ($(rel "$OUT")/edg-build.log)"; }
[[ -x "$EDG_BUILD/bin/cpfe" ]] || die "EDG build produced no $(rel "$EDG_BUILD")/bin/cpfe"

# ---- 3b. cerastes front end (EDG with the cerastes configuration + back end) ----
# Top-level CMake project; reuses the headers generated by the stock EDG build
# of the same tree.  Configured on every run (cheap): the tree, headers and
# output directory are passed explicitly so a stale cache cannot keep old ones.
log "cmake -S . -B $(rel "$CERASTES_BUILD")"
cmake -S "$ROOT" -B "$CERASTES_BUILD" -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  "-DCERASTES_EDG_SRC=$EDG_SRC" "-DCERASTES_EDG_GEN=$EDG_BUILD/src/includes" \
  "-DCERASTES_OUTPUT_DIR=$OUT" \
  > "$OUT/cerastes-configure.log" 2>&1 \
  || { tail -20 "$OUT/cerastes-configure.log"; die "cerastes configure failed ($(rel "$OUT")/cerastes-configure.log)"; }
log "ninja -C $(rel "$CERASTES_BUILD")"
ninja -C "$CERASTES_BUILD" > "$OUT/cerastes-build.log" 2>&1 \
  || { tail -30 "$OUT/cerastes-build.log"; die "cerastes build failed ($(rel "$OUT")/cerastes-build.log)"; }
[[ -x "$OUT/cerastes-fe" ]] || die "cerastes build produced no $(rel "$OUT")/cerastes-fe"
# Both builds succeeded: record which tree state they were built from.
python3 "$ROOT/scripts/edg_tree.py" stamp "$EDG_SRC" > "$EDG_STAMP.part"
mv "$EDG_STAMP.part" "$EDG_STAMP"

# ---- 4. Silicon -----------------------------------------------------------------
SILICON_SRC="$ROOT/third_party/silicon"
find_silicon_jar() {
  find "$SILICON_SRC/target" -maxdepth 2 -path '*/scala-*/silicon.jar' -print -quit 2>/dev/null || true
}
SILICON_JAR="$(find_silicon_jar)"
silicon_stamp() {
  # The jar is current when both checkouts are clean and at the stamped commits.
  printf 'silicon %s\nsilver %s\ndirty %s\njdk %s\n' \
    "$(git -C "$SILICON_SRC" rev-parse HEAD)" \
    "$(git -C "$SILICON_SRC/silver" rev-parse HEAD)" \
    "$(git -C "$SILICON_SRC" status --porcelain --ignore-submodules=none | wc -l | tr -d ' ')" \
    "$JDK_FORMULA"
}
STAMP_FILE="$BUILD/silicon.stamp"
if [[ $FORCE_SILICON -eq 1 || -z "$SILICON_JAR" || ! -f "$STAMP_FILE" \
      || "$(silicon_stamp)" != "$(cat "$STAMP_FILE")" ]]; then
  log "sbt assembly (third_party/silicon, JDK: $JDK_FORMULA)"
  (cd "$SILICON_SRC" && sbt --batch assembly) > "$BUILD/silicon-build.log" 2>&1 \
    || { tail -30 "$BUILD/silicon-build.log"; die "Silicon build failed (build/silicon-build.log)"; }
  SILICON_JAR="$(find_silicon_jar)"
  [[ -n "$SILICON_JAR" ]] || die "sbt assembly produced no silicon.jar"
  silicon_stamp > "$STAMP_FILE"
else
  log "silicon.jar is current (${SILICON_JAR#"$ROOT"/})"
fi

# ---- 5. Z3 ----------------------------------------------------------------------
Z3_DIR="$TOOLS/$Z3_ASSET"
Z3_EXE="$Z3_DIR/bin/z3"
if [[ ! -x "$Z3_EXE" ]]; then
  [[ $BUILD_ONLY -eq 0 ]] || die "pinned Z3 is missing; run scripts/bootstrap.sh"
  log "downloading Z3 $Z3_VERSION"
  zip="$TOOLS/$Z3_ASSET.zip"
  curl --fail --silent --show-error --location "$Z3_URL" --output "$zip.part"
  echo "$Z3_SHA256  $zip.part" | shasum -a 256 -c --status \
    || { rm -f "$zip.part"; die "Z3 download failed its SHA-256 check"; }
  mv "$zip.part" "$zip"
  rm -rf "$Z3_DIR"
  unzip -q "$zip" -d "$TOOLS"
  rm -f "$zip"
fi
"$Z3_EXE" --version | grep -q "Z3 version $Z3_VERSION" || die "$Z3_EXE is not Z3 $Z3_VERSION"

# ---- 5b. Documentation dependencies (doc/node_modules) ------------------------
# `npm ci` installs exactly what doc/package-lock.json pins.  The stamp records
# package.json, the lockfile and the node major it was installed with; the
# installed tree is also compared with the lockfile (see doc_modules_match_lock).
DOC_DIR="$ROOT/doc"
DOC_STAMP="$BUILD/doc-npm.stamp"
command -v node >/dev/null || die "node is missing; run scripts/bootstrap.sh"
NODE_MAJOR="$(node -p 'process.versions.node.split(".")[0]')"
WANT_NODE_MAJOR="$(tr -d '[:space:]' < "$DOC_NODE_VERSION_FILE")"
if [[ "$NODE_MAJOR" != "$WANT_NODE_MAJOR" ]]; then
  warn "node $(node -v) found; doc/.node-version asks for $WANT_NODE_MAJOR (CI uses it)"
fi
doc_stamp() {
  # package.json and the lockfile, plus the node major.
  printf 'lock %s\npackage %s\nnode %s\n' \
    "$(shasum -a 256 "$DOC_DIR/package-lock.json" | awk '{print $1}')" \
    "$(shasum -a 256 "$DOC_DIR/package.json" | awk '{print $1}')" "$NODE_MAJOR"
}
# npm writes node_modules/.package-lock.json when it installs; it equals the
# lockfile's content for the installed tree only if nothing was changed by hand
# (compared on the "packages" map, which both files carry).
doc_modules_match_lock() {
  [[ -f "$DOC_DIR/node_modules/.package-lock.json" ]] || return 1
  node -e '
    const fs = require("fs");
    const lock = JSON.parse(fs.readFileSync(process.argv[1], "utf8")).packages;
    const hidden = JSON.parse(fs.readFileSync(process.argv[2], "utf8")).packages;
    // The hidden lockfile has no root entry ("") and omits packages that are not installed.
    for (const [k, v] of Object.entries(hidden)) {
      const l = lock[k];
      if (!l || l.version !== v.version || l.resolved !== v.resolved || l.integrity !== v.integrity) process.exit(1);
    }
    for (const [k, l] of Object.entries(lock)) {
      if (k !== "" && !l.optional && !l.peer && !hidden[k]) process.exit(1);
    }
  ' "$DOC_DIR/package-lock.json" "$DOC_DIR/node_modules/.package-lock.json"
}
if [[ -d "$DOC_DIR/node_modules" && -f "$DOC_STAMP" && "$(doc_stamp)" == "$(cat "$DOC_STAMP")" ]] \
   && doc_modules_match_lock; then
  log "doc dependencies are current (doc/node_modules)"
else
  [[ $BUILD_ONLY -eq 0 ]] || die "doc/node_modules is missing or stale; run scripts/bootstrap.sh"
  log "npm ci (doc/)"
  (cd "$DOC_DIR" && npm ci --ignore-scripts --no-audit --no-fund) > "$BUILD/doc-npm.log" 2>&1 \
    || { tail -20 "$BUILD/doc-npm.log"; die "npm ci failed (build/doc-npm.log)"; }
  doc_stamp > "$DOC_STAMP"
fi
ANTORA_VERSION="$(cd "$DOC_DIR" && npx --no-install antora --version 2>/dev/null | head -1 | sed 's/^@antora\/cli: //')" \
  || die "antora is not runnable from doc/node_modules"

# ---- 6. build/env.sh --------------------------------------------------------------
# EDG has no standard library of its own on macOS; it parses the system libc++
# and SDK headers while emulating the installed clang.
SCRAPE="$EDG_SRC/dev_tools/bin/edg-scrape-compiler"
EDG_CLANG_INCL_SCRAPE="$(python3 "$SCRAPE" clang --lang c++ includes)"
EDG_CLANG_CINCL_SCRAPE="$(python3 "$SCRAPE" clang --lang c includes)"
EDG_CLANG_VER_SCRAPE="$(python3 "$SCRAPE" clang version)"
# EDG_BASE as chosen by EDG's CMake (its host default for the preset).
EDG_BASE="$(sed -n 's/^export EDG_BASE="\(.*\)"$/\1/p' "$EDG_BUILD/environment.sh")"
[[ -d "$EDG_BASE" ]] || die "cannot determine EDG_BASE from $(rel "$EDG_BUILD")/environment.sh"

# Native compilers for `// expect: native-ok` tests: Apple clang, and the
# highest-numbered Homebrew GCC (gcc-NN with a g++-NN of the same number) if any.
CLANG_CC="$(xcrun -f clang)"
CLANG_CXX="$(xcrun -f clang++)"
"$CLANG_CXX" --version 2>&1 | head -1 | grep -q 'Apple clang' || die "$CLANG_CXX is not Apple clang"
GCC_CC=""
GCC_CXX=""
gcc_best=0
IFS=: read -r -a path_dirs <<<"$PATH"
for dir in "${path_dirs[@]}"; do
  for cand in "$dir"/gcc-[0-9]*; do
    [[ -x "$cand" ]] || continue
    num="${cand##*/gcc-}"
    [[ "$num" =~ ^[0-9]+$ ]] || continue
    (( num > gcc_best )) || continue
    gxx="$(command -v "g++-$num" || true)"
    [[ -n "$gxx" ]] || continue
    "$cand" --version 2>&1 | head -1 | grep -q 'GCC' || continue
    "$gxx" --version 2>&1 | head -1 | grep -q 'GCC' || continue
    gcc_best="$num"; GCC_CC="$cand"; GCC_CXX="$gxx"
  done
done

cat > "$BUILD/env.sh" <<EOF
# Generated by scripts/bootstrap.sh; do not edit.  Source it to use the
# pinned toolchain: . build/env.sh
export CERASTES_ROOT="$ROOT"
export CERASTES_JAVA="$JAVA_HOME/bin/java"
export SILICON_JAR="$SILICON_JAR"
export Z3_EXE="$Z3_EXE"
export CERASTES_EDG_KIND="$EDG_KIND"
export CERASTES_EDG_SRC_ACTIVE="$EDG_SRC"
export CERASTES_EDG_GEN="$EDG_BUILD/src/includes"
export CERASTES_EDG_STAMP="$EDG_STAMP"
export CERASTES_INCLUDE_DIR="$ROOT/include"
export CERASTES_CLANG_CC="$CLANG_CC"
export CERASTES_CLANG_CXX="$CLANG_CXX"
export CERASTES_GCC_CC="$GCC_CC"
export CERASTES_GCC_CXX="$GCC_CXX"
export EDG_BUILD_DIR="$EDG_BUILD"
export CPFE="$EDG_BUILD/bin/cpfe"
export CERASTES_FE="$OUT/cerastes-fe"
export EDG_BASE="$EDG_BASE"
export EDG_CLANG_INCL_SCRAPE="$EDG_CLANG_INCL_SCRAPE"
export EDG_CLANG_CINCL_SCRAPE="$EDG_CLANG_CINCL_SCRAPE"
export EDG_CLANG_VER_SCRAPE="$EDG_CLANG_VER_SCRAPE"
EOF

log "toolchain ready; source build/env.sh to use it"
printf '    %-8s %s\n' \
  java "$("$JAVA_HOME/bin/java" -version 2>&1 | head -1)" \
  z3 "$("$Z3_EXE" --version)" \
  edg "$EDG_KIND $EDG_SRC" \
  cpfe "$(rel "$EDG_BUILD")/bin/cpfe" \
  cerastes-fe "$(rel "$OUT")/cerastes-fe" \
  clang "$CLANG_CXX" \
  gcc "${GCC_CXX:-<none>}" \
  silicon "$(rel "$SILICON_JAR")" \
  node "$(node -v), npm $(npm -v)" \
  antora "$ANTORA_VERSION"
