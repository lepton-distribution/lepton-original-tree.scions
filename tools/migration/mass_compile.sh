#!/usr/bin/env bash
# mass_compile.sh — compilation de masse GCC du périmètre actif (ETAPE-4 tâche 4).
# Lanceur de mass_compile.py (options : mass_compile.py --help). Code de retour 1 tant que le
# périmètre actif ne compile pas à 100 %.
set -euo pipefail
here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=/dev/null
[ -n "${LEPTON_TRUNK:-}" ] || source "$here/../../scripts/lepton-env.sh" >/dev/null
exec python3 "$here/mass_compile.py" "$@"
