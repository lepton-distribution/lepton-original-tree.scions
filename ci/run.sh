#!/usr/bin/env bash
# ci/run.sh — non-régression Lepton (étape 3 ; ORCHESTRATION §5 : vert avant chaque commit).
# Configure et construit les presets hôte et QEMU (hard-float, soft-float), puis ctest -L host,
# -L smoke, -L kal.
# Code de retour non nul au premier échec. Lancer depuis n'importe où dans le rootstock.
#   ci/run.sh            tous les presets
#   ci/run.sh --no-kal   sans le banc KAL (le temps de sa construction à l'étape 3)
set -euo pipefail

here="$(cd "$(dirname "$0")/.." && pwd)"
# argument explicite : sinon lepton-env.sh lirait les options de ce script ($1)
# shellcheck source=/dev/null
source "$here/scripts/lepton-env.sh" "$here" >/dev/null

run_kal=1
for arg in "$@"; do
  case "$arg" in
    --no-kal) run_kal=0 ;;
    *) echo "option inconnue : $arg" >&2; exit 2 ;;
  esac
done

step() { printf '\n== %s\n' "$*"; }

cd "$LEPTON_TRUNK"

step "preset host : configuration, build, ctest -L host"
cmake --preset host >/dev/null
cmake --build --preset host
ctest --preset host -L host --no-tests=error

# hard-float (cible) puis soft-float (chemin sans FPU, jusqu'aux cœurs M3/M0 de l'étape 6)
for preset in qemu-mps2-an386-embos qemu-mps2-an386-embos-soft; do
  step "preset $preset : configuration, build, ctest -L smoke"
  cmake --preset "$preset" >/dev/null
  cmake --build --preset "$preset"
  ctest --preset "$preset" -L smoke --no-tests=error

  if [ "$run_kal" = 1 ]; then
    step "banc KAL ($preset) : ctest -L kal"
    ctest --preset "$preset" -L kal --no-tests=error
  fi
done

step "trunk : aucun fichier régulier"
regular="$(find "$LEPTON_TRUNK" -type f ! -name .scion.grafted.list | head -5)"
if [ -n "$regular" ]; then
  echo "fichiers réguliers dans le trunk :" >&2
  echo "$regular" >&2
  exit 1
fi

echo
echo "ci/run.sh : vert"
