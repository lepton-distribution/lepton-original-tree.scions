#!/usr/bin/env bash
# install-debian.sh — Prérequis de build Lepton sous Debian x86_64 (stable).
# Utilisable dans trois contextes identiques :
#   - depuis ci/Dockerfile (RUN ./install-debian.sh) : environnement de référence,
#   - sur un runner CI Debian,
#   - sur une machine ou VM Debian native (dans ce cas, --with-debug-tools ajoute
#     OpenOCD pour le flash/debug local avec accès USB).
#
# Usage : ./install-debian.sh [--with-debug-tools] [--with-riscv]
#   --with-debug-tools : ajoute openocd + règles udev (hors conteneur uniquement)
#   --with-riscv       : toolchain et QEMU RISC-V (portage reporté, annexe)

set -euo pipefail
export DEBIAN_FRONTEND=noninteractive

RISCV=0
DEBUG_TOOLS=0
for arg in "$@"; do
  case "$arg" in
    --with-riscv)       RISCV=1 ;;
    --with-debug-tools) DEBUG_TOOLS=1 ;;
    *) echo "option inconnue: $arg" >&2; exit 2 ;;
  esac
done

log()  { printf '\n== %s\n' "$*"; }
SUDO=""
[ "$(id -u)" -ne 0 ] && SUDO="sudo"

log "Mise à jour des index"
$SUDO apt-get update

# --- Base : build system, compilateur host, scripts -------------------------
log "Base (build-essential, cmake, ninja, python, git)"
$SUDO apt-get install -y --no-install-recommends \
  build-essential gcc g++ gdb clang \
  cmake ninja-build \
  git ca-certificates \
  python3 python3-pip pipx \
  libexpat1-dev \
  file bc

# --- scion : composition de l'arbre des sources Lepton (étape 0) -------------
# Épinglé sur le tag 0.5.0.1 (la tête de master lui est postérieure).
# pipx : seule voie propre sous PEP 668 ; commande déposée dans ~/.local/bin.
SCION_REF="0.5.0.1"
log "scion ${SCION_REF}"
pipx install --force "git+https://github.com/lepton-distribution/seed.scions.git@${SCION_REF}" \
  || echo "AVERTISSEMENT: installation de scion échouée — bloquant pour l'étape 0."
pipx ensurepath >/dev/null 2>&1 || true

# --- Outillage migration (audit, transformation, métriques) -----------------
log "Outillage migration (cloc, coccinelle)"
$SUDO apt-get install -y --no-install-recommends \
  cloc coccinelle

# --- Toolchain croisée ARM (étapes 2 à 7) -------------------------------------
log "Toolchain ARM (gcc-arm-none-eabi + newlib)"
$SUDO apt-get install -y --no-install-recommends \
  gcc-arm-none-eabi binutils-arm-none-eabi \
  libnewlib-arm-none-eabi \
  gdb-multiarch
# newlib-nano est fourni par libnewlib-arm-none-eabi (libc_nano.a, nano.specs) : pas de paquet séparé.
if arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 --specs=nano.specs \
     -print-file-name=libc_nano.a | grep -q '^/'; then
  echo "newlib-nano : OK"
else
  echo "AVERTISSEMENT: libc_nano.a introuvable pour cortex-m4 hard-float (newlib-nano)."
fi

# --- QEMU (socle mps2-an386, banc KAL) ----------------------------------------
log "QEMU (system-arm)"
$SUDO apt-get install -y --no-install-recommends qemu-system-arm
# test réseau (tests/net_qemu.py, label net) : tap dans un espace de noms utilisateur (unshare :
# util-linux, essentiel), ip, ping ; espaces de noms utilisateur non privilégiés requis
$SUDO apt-get install -y --no-install-recommends iproute2 iputils-ping

# --- RISC-V (reporté, optionnel) ---------------------------------------------
if [ "$RISCV" = "1" ]; then
  log "Toolchain et QEMU RISC-V"
  $SUDO apt-get install -y --no-install-recommends \
    gcc-riscv64-unknown-elf binutils-riscv64-unknown-elf picolibc-riscv64-unknown-elf \
    qemu-system-misc \
    || echo "AVERTISSEMENT: paquets RISC-V indisponibles — envisager xPack riscv-none-elf-gcc."
fi

# --- Graphify (optionnel : graphe de connaissances du dépôt pour Claude Code) --
# Parsing tree-sitter local (le code ne quitte pas la machine). Python >= 3.10 requis.
# Le nom du paquet PyPI et la procédure exacte sont à vérifier sur le dépôt officiel
# au moment de l'installation (projet récent, distribution en évolution) ;
# épingler la version retenue dans MIGRATION-STATUS.md.
log "Graphify (optionnel)"
pip3 install --break-system-packages graphify 2>/dev/null \
  || echo "AVERTISSEMENT: installation pip de Graphify non aboutie — suivre la procédure du dépôt officiel (github.com/macouen/graphify) ; étape non bloquante."

# --- Documentation (Doxygen) ------------------------------------------------
log "Documentation (doxygen, graphviz)"
$SUDO apt-get install -y --no-install-recommends doxygen graphviz

# --- Debug local avec USB (machine/VM native uniquement) --------------------
if [ "$DEBUG_TOOLS" = "1" ]; then
  log "Outils de debug locaux (openocd)"
  $SUDO apt-get install -y --no-install-recommends openocd
  if [ -d /etc/udev/rules.d ] && [ ! -f /.dockerenv ]; then
    echo "-> Vérifier les règles udev de la sonde (OpenOCD fournit 60-openocd.rules)."
  fi
fi

$SUDO apt-get clean
$SUDO rm -rf /var/lib/apt/lists/* 2>/dev/null || true

# --- Récapitulatif : versions à épingler dans MIGRATION-STATUS.md -----------
log "Versions installées (à consigner dans MIGRATION-STATUS.md)"
for c in gcc cmake ninja python3 scion cloc spatch \
         arm-none-eabi-gcc riscv64-unknown-elf-gcc \
         qemu-system-arm qemu-system-riscv32 doxygen openocd; do
  if command -v "$c" >/dev/null 2>&1; then
    if [ "$c" = scion ]; then v="$(scion version 2>/dev/null | tail -1)"; else v="$("$c" --version 2>/dev/null | head -1)"; fi
    printf '  %-26s %s\n' "$c" "$v"
  else
    printf '  %-26s (absent)\n' "$c"
  fi
done

echo
echo "Terminé."
