#!/usr/bin/env bash
# ci/run.sh — non-régression Lepton (étape 3 ; ORCHESTRATION §5 : vert avant chaque commit).
# Pipeline de l'étape 6 :
#   - tests unitaires des outils de migration ; garde audit_iar.py (zéro IAR-isme Lepton actif) ;
#   - preset hôte : build, ctest -L host ; même chose avec clang ($LEPTON_BUILD/host-clang, étape 8) ;
#   - matrice micro-noyau (embOS, FreeRTOS, étape 7) × machine :
#     presets QEMU (mps2-an386 hard-float et soft-float, mps2-an500 Cortex-M7) : build,
#     ctest -L smoke, -L net, -L kal ;
#     presets carte (F439/F429, F746, SAMD21, WL55) : build seul (pas d'exécution sans sonde) ;
#   - occupation mémoire de chaque exécutable (--print-memory-usage) archivée dans
#     $LEPTON_BUILD/ci/memoire.csv, échec au-delà de LEPTON_MEM_ALERT % (défaut 90) par région ;
#   - artefacts .elf/.bin/.map copiés dans $LEPTON_BUILD/ci/artefacts/<preset>/.
# Code de retour non nul au premier échec. Lancer depuis n'importe où dans le rootstock.
#   ci/run.sh            tous les presets
#   ci/run.sh --no-kal   sans le banc KAL
set -euo pipefail
# les tests Python importent leurs voisins (net_qemu → smoke_lsh) : sans cela, le cache
# __pycache__ serait écrit en fichier régulier dans le trunk (contrôle final en échec)
export PYTHONDONTWRITEBYTECODE=1

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

mem_alert="${LEPTON_MEM_ALERT:-90}"
ci_out="$LEPTON_BUILD/ci"
rm -rf "$ci_out"
mkdir -p "$ci_out/artefacts"

# build_cible <preset> : configuration et build d'un preset ARM ; les .elf sont supprimés avant le
# build pour forcer l'édition de liens, donc le relevé --print-memory-usage de chaque exécutable ;
# relevé archivé et comparé au seuil, artefacts copiés.
build_cible() {
  local preset="$1" dir="$LEPTON_BUILD/$1"
  step "preset $preset : configuration, build, occupation mémoire (seuil $mem_alert %)"
  cmake --preset "$preset" >/dev/null
  find "$dir" -name '*.elf' -delete 2>/dev/null || true
  cmake --build --preset "$preset" 2>&1 | tee "$ci_out/build-$preset.log"
  local nb_elf
  nb_elf="$(find "$dir" -name '*.elf' | wc -l)"
  python3 "$here/ci/memoire.py" --preset "$preset" --log "$ci_out/build-$preset.log" \
    --csv "$ci_out/memoire.csv" --seuil "$mem_alert" --attendus "$nb_elf"
  mkdir -p "$ci_out/artefacts/$preset"
  find "$dir" -path '*/CMakeFiles' -prune -o \( -name '*.elf' -o -name '*.bin' -o -name '*.map' \) \
    -exec cp -p {} "$ci_out/artefacts/$preset/" \;
}

cd "$LEPTON_TRUNK"

step "outils de migration : tests unitaires (transform_iar.py, kal_split.py, axes_kernel_core.py)"
python3 -m unittest discover -s "$here/tools/migration/tests"

step "garde audit_iar.py : aucun IAR-isme dans le code Lepton du périmètre actif"
python3 "$here/tools/migration/audit_iar.py" --garde

step "preset host : configuration, build, ctest -L host"
cmake --preset host >/dev/null
cmake --build --preset host
ctest --preset host -L host --no-tests=error

step "preset host, second compilateur (clang) : configuration, build, ctest -L host"
cmake --preset host -B "$LEPTON_BUILD/host-clang" -DCMAKE_C_COMPILER=clang >/dev/null
cmake --build "$LEPTON_BUILD/host-clang"
ctest --test-dir "$LEPTON_BUILD/host-clang" -L host --no-tests=error

# matrice micro-noyau × machine (étape 7) : <machine>-<micro-noyau>
kernels="embos freertos"
qemu_machines="qemu-mps2-an386 qemu-mps2-an386@soft qemu-mps2-an500"
board_machines="nucleo-f439zi stm32f746g-disco samd21-xplained-pro nucleo-wl55jc1"
# preset d'une machine : <machine>-<micro-noyau>[-<variante>] (an386@soft → an386-<µn>-soft)
preset_of() {
  local machine="${1%@*}" variant=""
  [ "$machine" != "$1" ] && variant="-${1#*@}"
  echo "$machine-$2$variant"
}

# M4F hard-float (cible) puis soft-float (chemin sans FPU), M7 (étape 6), sous chaque micro-noyau
for kernel in $kernels; do
for machine in $qemu_machines; do
  preset="$(preset_of "$machine" "$kernel")"
  build_cible "$preset"

  step "fumée ($preset) : ctest -L smoke"
  ctest --preset "$preset" -L smoke --no-tests=error

  step "réseau ($preset) : ctest -L net (ping, ftpd ; tap en espace de noms)"
  ctest --preset "$preset" -L net --no-tests=error

  if [ "$run_kal" = 1 ]; then
    step "banc KAL ($preset) : ctest -L kal"
    ctest --preset "$preset" -L kal --no-tests=error
  fi
done
done

# cartes (étapes 5 à 7) : build seul, les tests board exigent la sonde (ctest -L board, à la main)
for kernel in $kernels; do
for machine in $board_machines; do
  build_cible "$(preset_of "$machine" "$kernel")"
done
done

step "trunk : aucun fichier régulier"
regular="$(find "$LEPTON_TRUNK" -type f ! -name .scion.grafted.list | head -5)"
if [ -n "$regular" ]; then
  echo "fichiers réguliers dans le trunk :" >&2
  echo "$regular" >&2
  exit 1
fi

echo
echo "occupation mémoire : $ci_out/memoire.csv ; artefacts : $ci_out/artefacts/"
echo "ci/run.sh : vert"
