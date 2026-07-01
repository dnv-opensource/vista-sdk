#!/usr/bin/env bash
#
# Bump the vista-sdk version across all SDKs (C#, JS, Python) in one shot,
# with two modes: a stable "release" and a "preview".
#
# Usage:
#   bump-version.sh --current        # show current version + which CI mode is active
#   bump-version.sh release          # finalize current preview: X.Y.Z-preview -> X.Y.Z
#   bump-version.sh preview          # open next preview: X.Y.Z -> X.Y.(Z+1)-preview
#   bump-version.sh preview minor    # next preview bumping minor: X.Y.Z -> X.(Y+1).0-preview
#   bump-version.sh preview major    # next preview bumping major: X.Y.Z -> (X+1).0.0-preview
#   bump-version.sh release <X.Y.Z>  # explicit release version
#   bump-version.sh preview <X.Y.Z>  # explicit preview version (X.Y.Z -> X.Y.Z-preview)
#   bump-version.sh --help
#
# release/preview with no argument derive the target from the CURRENT version;
# re-running in the same mode is a no-op (e.g. preview when already a preview).
#
# Source-of-truth version locations (CI appends the run number at publish time
# in preview mode; release mode removes it):
#   1. js/packages/vista-sdk/package.json                -> "version"
#   2. js/packages/vista-sdk-experimental/package.json   -> "version" + peerDep ">=X.Y.Z-0"
#   3. .github/workflows/build-python.yml                -> BASE_VERSION (+ VERSION revision)
#   4. .github/workflows/build-csharp.yml                -> /p:Version=...   (revision)
#   +  .github/workflows/build-js.yml                    -> npm version revision + dist-tag
#
# The "revision" is the CI run-number suffix (-${{ steps.rev.outputs.count }} /
# -${REV} / .${{ steps.rev.outputs.count }}). It is REMOVED for a release so the
# published version is a clean X.Y.Z, and RE-INTRODUCED on the next preview bump.
#
set -euo pipefail

REPO_ROOT="$(git rev-parse --show-toplevel 2>/dev/null || true)"
if [[ -z "${REPO_ROOT}" ]]; then
  echo "error: must be run inside the vista-sdk git repository" >&2
  exit 1
fi
cd "${REPO_ROOT}"

JS_MAIN="js/packages/vista-sdk/package.json"
JS_EXP="js/packages/vista-sdk-experimental/package.json"
PY_WF=".github/workflows/build-python.yml"
CS_WF=".github/workflows/build-csharp.yml"
JS_WF=".github/workflows/build-js.yml"

# Exact CI revision tokens (kept here so the workflows stay the single source of truth).
CS_REV='-${{ steps.rev.outputs.count }}'
JS_REV='.${{ steps.rev.outputs.count }}'
PY_REV='-${REV}'

require_node() {
  command -v node >/dev/null 2>&1 || { echo "error: node is required but not found on PATH" >&2; exit 1; }
}

current_version() {
  require_node
  node -p "require('./${JS_MAIN}').version"
}

# Detect whether the CI workflows currently carry the revision suffix.
ci_mode() {
  if grep -Eq '/p:Version=[^ ]*-\$\{\{ steps\.rev\.outputs\.count \}\}' "${CS_WF}"; then
    echo "preview"
  else
    echo "release"
  fi
}

print_current() {
  local cur core
  cur="$(current_version)"
  core="${cur%%-*}"
  echo "Current version: ${cur}  (numeric core: ${core})"
  echo "CI publish mode: $(ci_mode)  (preview = run-number appended; release = clean version)"
  echo
  echo "Source-of-truth locations:"
  grep -nH '"version"' "${JS_MAIN}" | head -1
  grep -nH '"version"' "${JS_EXP}" | head -1
  grep -nH 'dnv-vista-sdk": ">=' "${JS_EXP}" | head -1
  grep -nH 'BASE_VERSION:' "${PY_WF}" | head -1
  grep -nH '/p:Version=' "${CS_WF}" | head -1
  grep -nH 'npm version' "${JS_WF}"
}

usage() {
  sed -n '3,30p' "$0" | sed 's/^#$//; s/^# //'
}

die() { echo "error: $*" >&2; exit 1; }

# --- arg parsing -------------------------------------------------------------
[[ $# -ge 1 ]] || { usage; exit 1; }

MODE=""
INPUT=""   # optional: an explicit version, or (preview only) a patch|minor|major level
case "$1" in
  -h|--help)        usage; exit 0 ;;
  --current|--show) print_current; exit 0 ;;
  release|preview)
    MODE="$1"; shift
    [[ $# -le 1 ]] || die "'${MODE}' takes at most one argument"
    [[ $# -eq 1 ]] && INPUT="$1"
    ;;
  *)
    INPUT="$1"
    # Infer: a prerelease suffix => preview, a clean X.Y.Z => release.
    if [[ "${INPUT}" == *-* ]]; then MODE="preview"; else MODE="release"; fi
    ;;
esac

require_node
OLD_VERSION="$(current_version)"
OLD_CORE="${OLD_VERSION%%-*}"

# Increment one level of a clean X.Y.Z core.
bump_level() {
  local core="$1" level="$2" ma mi pa IFS=.
  set -- $core
  ma="$1"; mi="$2"; pa="$3"
  case "${level}" in
    major) echo "$((ma + 1)).0.0" ;;
    minor) echo "${ma}.$((mi + 1)).0" ;;
    patch) echo "${ma}.${mi}.$((pa + 1))" ;;
  esac
}

# --- resolve the target version per mode -------------------------------------
# Default behaviour derives the target from the CURRENT version:
#   release : X.Y.Z-preview -> X.Y.Z         (drop the suffix; already-release = no change)
#   preview : X.Y.Z         -> X.Y.(Z+1)-preview   (already-preview = no change)
# An explicit version, or a patch|minor|major level (preview only), overrides this.
case "${MODE}" in
  release)
    if [[ -n "${INPUT}" ]]; then
      [[ "${INPUT}" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]] || \
        die "release version must be a clean MAJOR.MINOR.PATCH (no prerelease suffix); got '${INPUT}'"
      NEW_VERSION="${INPUT}"
    else
      NEW_VERSION="${OLD_CORE}"            # finalize current core: drop any -preview suffix
    fi
    ;;
  preview)
    if [[ -z "${INPUT}" ]]; then
      if [[ "${OLD_VERSION}" == *-* ]]; then
        NEW_VERSION="${OLD_VERSION}"       # already a preview -> no change
      else
        NEW_VERSION="$(bump_level "${OLD_CORE}" patch)-preview"
      fi
    elif [[ "${INPUT}" =~ ^(patch|minor|major)$ ]]; then
      NEW_VERSION="$(bump_level "${OLD_CORE}" "${INPUT}")-preview"
    elif [[ "${INPUT}" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
      NEW_VERSION="${INPUT}-preview"
    elif [[ "${INPUT}" =~ ^[0-9]+\.[0-9]+\.[0-9]+-[0-9A-Za-z.]+$ ]]; then
      NEW_VERSION="${INPUT}"
    else
      die "'${INPUT}' is not a valid version or level (use patch|minor|major, or X.Y.Z[-suffix])"
    fi
    ;;
esac

NEW_CORE="${NEW_VERSION%%-*}"

# No-op short-circuit (e.g. preview when already a preview, release when already released).
if [[ "${NEW_VERSION}" == "${OLD_VERSION}" && "$(ci_mode)" == "${MODE}" ]]; then
  echo "Already ${OLD_VERSION} in ${MODE} mode; no change."
  exit 0
fi

echo "Mode:    ${MODE}"
echo "Version: ${OLD_VERSION}  ->  ${NEW_VERSION}"
echo

# --- version strings: package.json (1 & 2) -----------------------------------
set_pkg_version() {
  local file="$1"
  node -e '
    const fs = require("fs");
    const f = process.argv[1], v = process.argv[2];
    const raw = fs.readFileSync(f, "utf8");
    const eol = raw.includes("\r\n") ? "\r\n" : "\n";
    const trailing = raw.endsWith("\n") ? eol : "";
    const obj = JSON.parse(raw);
    obj.version = v;
    fs.writeFileSync(f, JSON.stringify(obj, null, 4) + trailing);
  ' "${file}" "${NEW_VERSION}"
}
set_pkg_version "${JS_MAIN}"
set_pkg_version "${JS_EXP}"

# experimental peerDependency floor ">=X.Y.Z-0"
node -e '
  const fs = require("fs");
  const f = process.argv[1], core = process.argv[2];
  const raw = fs.readFileSync(f, "utf8");
  const eol = raw.includes("\r\n") ? "\r\n" : "\n";
  const trailing = raw.endsWith("\n") ? eol : "";
  const obj = JSON.parse(raw);
  if (obj.peerDependencies && obj.peerDependencies["dnv-vista-sdk"]) {
    obj.peerDependencies["dnv-vista-sdk"] =
      obj.peerDependencies["dnv-vista-sdk"].replace(/>=\d+\.\d+\.\d+-0/, ">=" + core + "-0");
  }
  fs.writeFileSync(f, JSON.stringify(obj, null, 4) + trailing);
' "${JS_EXP}" "${NEW_CORE}"

# --- python BASE_VERSION (3) -------------------------------------------------
sed -i -E "s/^([[:space:]]*BASE_VERSION:[[:space:]]*\")[^\"]*(\")/\1${NEW_VERSION}\2/" "${PY_WF}"

# --- CI revision toggle (csharp, python, js) ---------------------------------
# Compute the per-mode suffixes / tokens.
if [[ "${MODE}" == "preview" ]]; then
  cs_suffix="${CS_REV}"
  py_suffix="${PY_REV}"
  js_npmver="\$(node -p \"require('./package.json').version\")${JS_REV}"
  js_tag="preview"
else
  cs_suffix=""
  py_suffix=""
  js_npmver="\$(node -p \"require('./package.json').version\") --allow-same-version"
  js_tag="latest"
fi

# C#: rewrite the whole /p:Version=... token (it is the last token on the line).
sed -i -E "s#/p:Version=.*#/p:Version=${NEW_VERSION}${cs_suffix}#" "${CS_WF}"

# Python: rewrite the VERSION="..." composition line.
sed -i -E "s#VERSION=\"\\\$\\{BASE_VERSION\\}[^\"]*\"#VERSION=\"\${BASE_VERSION}${py_suffix}\"#" "${PY_WF}"

# JS: rewrite the `npm version <...>;` token and the publish dist-tag.
sed -i -E "s#npm version [^;]*;#npm version ${js_npmver};#g" "${JS_WF}"
sed -i -E "s#--tag (preview|latest)#--tag ${js_tag}#g" "${JS_WF}"

# --- verify ------------------------------------------------------------------
echo "Updated files:"
fail=0
check() {
  local label="$1" file="$2" pattern="$3"
  if grep -Eq -e "${pattern}" "${file}"; then
    printf '  ok   %-14s %s\n' "${label}" "${file}"
  else
    printf '  FAIL %-14s %s  (expected /%s/)\n' "${label}" "${file}" "${pattern}" >&2
    fail=1
  fi
}
esc_ver="$(printf '%s' "${NEW_VERSION}" | sed -E 's/[.+]/\\&/g')"
esc_core="$(printf '%s' "${NEW_CORE}" | sed -E 's/[.+]/\\&/g')"

check "js"          "${JS_MAIN}" "\"version\": \"${esc_ver}\""
check "js-exp"      "${JS_EXP}"  "\"version\": \"${esc_ver}\""
check "js-exp-peer" "${JS_EXP}"  ">=${esc_core}-0"
check "python-base" "${PY_WF}"   "BASE_VERSION: \"${esc_ver}\""
check "csharp"      "${CS_WF}"   "/p:Version=${esc_ver}"

if [[ "${MODE}" == "preview" ]]; then
  check "csharp-rev"  "${CS_WF}" "/p:Version=${esc_ver}-\\\$\{\{ steps\.rev\.outputs\.count \}\}"
  check "python-rev"  "${PY_WF}" 'VERSION="\$\{BASE_VERSION\}-\$\{REV\}"'
  check "js-rev"      "${JS_WF}" 'npm version .*\.\$\{\{ steps\.rev\.outputs\.count \}\};'
  check "js-tag"      "${JS_WF}" "--tag preview"
else
  check "csharp-norev" "${CS_WF}" "/p:Version=${esc_ver}\$"
  check "python-norev" "${PY_WF}" 'VERSION="\$\{BASE_VERSION\}"'
  check "js-tag"       "${JS_WF}" "--tag latest"
  if grep -Eq '\.\$\{\{ steps\.rev\.outputs\.count \}\}' "${JS_WF}"; then
    echo "  FAIL js-rev        ${JS_WF}  (run-number suffix should be absent in release mode)" >&2
    fail=1
  else
    printf '  ok   %-14s %s\n' "js-norev" "${JS_WF}"
  fi
fi

echo
if [[ "${fail}" -ne 0 ]]; then
  echo "One or more files did not update as expected. Review 'git diff'." >&2
  exit 1
fi

# --- next-step guidance ------------------------------------------------------
if [[ "${MODE}" == "release" ]]; then
  cat <<EOF
Release ${NEW_VERSION} set across all SDKs (CI run-number removed).

Next steps (release):
  git commit -am "release: ${NEW_VERSION}"
  git tag -a v${NEW_VERSION} -m "Release ${NEW_VERSION}"
  git push && git push origin v${NEW_VERSION}

Then open the next preview:
  $0 preview
EOF
else
  # Soft, non-blocking reminder: was the previous release tagged/deployed?
  prev_core="${OLD_VERSION%%-*}"
  if [[ "${OLD_VERSION}" != *-* ]]; then
    if git rev-parse "v${OLD_VERSION}" >/dev/null 2>&1; then
      note="previous release v${OLD_VERSION} is tagged."
    else
      note="previous version ${OLD_VERSION} looks like a release but git tag v${OLD_VERSION} was not found — did you tag/deploy it?"
    fi
  else
    note="previous version was a preview (${OLD_VERSION})."
  fi
  cat <<EOF
Preview ${NEW_VERSION} set across all SDKs (CI run-number re-introduced).
Note: ${note}

Next steps (preview):
  git commit -am "chore: ${NEW_VERSION}"
  # push to main to publish a preview build (deployment optional)
EOF
fi
