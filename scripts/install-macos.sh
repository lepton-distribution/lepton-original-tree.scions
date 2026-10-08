#!/usr/bin/env bash
# install-macos.sh — Prérequis de build Lepton sous macOS (pendant de install-debian.sh, étape 9).
# Mac Intel (x86_64) : hôte visé par le plan. Apple Silicon (arm64) : le script s'y adapte
# (préfixe Homebrew, archive de la toolchain), mais le build de Lepton n'y a pas été validé.
#
# Usage : ./install-macos.sh [--with-debug-tools] [--toolchain-dir <répertoire>]
#                            [--pkg-manager macports|brew]
#   --with-debug-tools : ajoute OpenOCD et gdb avec Python (lien gdb-multiarch ; étape 10)
#   --toolchain-dir    : où déposer la toolchain ARM (défaut : $LEPTON_TOOLCHAIN_DIR, sinon ~/opt)
#   --pkg-manager      : gestionnaire de paquets ; défaut : MacPorts sur Intel, Homebrew sur
#                        Apple Silicon, selon ce qui est installé
#
# Mac Intel : Homebrew n'y fournit plus de paquets binaires (Tier 3 depuis septembre 2026, tout
# serait compilé sur place) ; MacPorts est donc le choix par défaut. MacPorts installe sous
# /opt/local et exige sudo (port install) ; Homebrew n'en demande pas.
#
# Ce que le script ne fait pas : installer MacPorts, Homebrew ou les outils de ligne de commande
# Xcode (il s'arrête en donnant la marche à suivre), modifier un fichier de démarrage du shell
# (il affiche la ligne PATH à ajouter), copier le paquet embOS (licence SFL, hors git).
# Rien d'i386 (retiré à l'étape 8) ; pas de règles udev (sans objet sous macOS).
# Compatible bash 3.2. Rejouable : ce qui est déjà en place n'est pas réinstallé.

set -euo pipefail

DEBUG_TOOLS=0
TOOLCHAIN_DIR="${LEPTON_TOOLCHAIN_DIR:-$HOME/opt}"
PKG=""
while [ $# -gt 0 ]; do
  case "$1" in
    --with-debug-tools) DEBUG_TOOLS=1; shift ;;
    --toolchain-dir)    TOOLCHAIN_DIR="${2:?--toolchain-dir : répertoire manquant}"; shift 2 ;;
    --pkg-manager)      PKG="${2:?--pkg-manager : macports ou brew}"; shift 2 ;;
    --with-riscv)       echo "--with-riscv : non pris en charge sous macOS (portage reporté)." >&2; exit 2 ;;
    *) echo "option inconnue: $1" >&2; exit 2 ;;
  esac
done

log()  { printf '\n== %s\n' "$*"; }
warn() { printf 'AVERTISSEMENT: %s\n' "$*" >&2; }
die()  { printf 'ERREUR: %s\n' "$*" >&2; exit 1; }

# --- Hôte ---------------------------------------------------------------------
[ "$(uname -s)" = "Darwin" ] || die "ce script est réservé à macOS (Debian : install-debian.sh)."
ARCH="$(uname -m)"
# Sous Rosetta, uname -m répond x86_64 sur un Mac Apple Silicon : la toolchain serait la mauvaise.
if [ "$(sysctl -n sysctl.proc_translated 2>/dev/null || echo 0)" = "1" ]; then
  die "shell lancé sous Rosetta : relancer depuis un terminal natif (arch -arm64 bash $0)."
fi
case "$ARCH" in
  x86_64) ARM_HOST="darwin-x86_64"
          ARM_SHA256="2d9e717dd4f7751d18936ae1365d25916534105ebcb7583039eff1092b824505" ;;
  arm64)  ARM_HOST="darwin-arm64"
          ARM_SHA256="c7c78ffab9bebfce91d99d3c24da6bf4b81c01e16cf551eb2ff9f25b9e0a3818"
          warn "Apple Silicon : prérequis installables, build de Lepton non validé sur cet hôte." ;;
  *) die "architecture non prise en charge : $ARCH" ;;
esac

xcode-select -p >/dev/null 2>&1 \
  || die "outils de ligne de commande Xcode absents : lancer « xcode-select --install », puis rejouer."
# Gestionnaire de paquets : choix explicite, sinon MacPorts sur Intel et Homebrew sur Apple
# Silicon s'ils sont installés, sinon l'autre.
have() { command -v "$1" >/dev/null 2>&1; }
if [ -z "$PKG" ]; then
  if [ "$ARCH" = "x86_64" ]; then
    if have port; then PKG=macports; elif have brew; then PKG=brew; fi
  else
    if have brew; then PKG=brew; elif have port; then PKG=macports; fi
  fi
fi
case "$PKG" in
  macports) have port || die "MacPorts absent : l'installer depuis https://www.macports.org/install.php, puis rejouer."
            PKG_PREFIX="/opt/local" ;;
  brew)     have brew || die "Homebrew absent : l'installer depuis https://brew.sh, puis rejouer."
            PKG_PREFIX="$(brew --prefix)"   # /usr/local (Intel) ou /opt/homebrew (Apple Silicon)
            [ "$ARCH" = "x86_64" ] && warn "Homebrew sur Intel : plus de paquets binaires, compilation sur place (long, sans garantie). MacPorts conseillé." ;;
  "")       if [ "$ARCH" = "x86_64" ]; then
              die "aucun gestionnaire de paquets : installer MacPorts (https://www.macports.org/install.php), puis rejouer."
            else
              die "aucun gestionnaire de paquets : installer Homebrew (https://brew.sh), puis rejouer."
            fi ;;
  *)        die "--pkg-manager : macports ou brew (reçu : $PKG)" ;;
esac
echo "gestionnaire de paquets : $PKG ($PKG_PREFIX)"

# pkg_install <nom brew>:<nom macports>… : installe ce qui manque, sans mettre à niveau l'existant.
# Un nom MacPorts peut porter des variantes (« qemu +target_arm »).
pkg_install() {
  local pair b m
  for pair in "$@"; do
    b="${pair%%:*}"; m="${pair#*:}"
    if [ "$PKG" = brew ]; then
      if brew list --formula "$b" >/dev/null 2>&1; then echo "  $b : déjà installé"; else brew install "$b"; fi
    else
      # shellcheck disable=SC2086  # variantes séparées du nom du port
      if port -q installed ${m%% *} 2>/dev/null | grep -q '(active)'; then
        echo "  ${m%% *} : déjà installé"
      else
        sudo port -N install $m
      fi
    fi
  done
}

# --- Base : build system, scripts ----------------------------------------------
# Compilateur hôte : Apple clang (outils Xcode). expat : fourni par le SDK de macOS.
log "Base (cmake, ninja, python, pipx)"
pkg_install cmake:cmake ninja:ninja
# python3 et pipx déjà présents (Python.org, ou autre gestionnaire) : conservés, sinon un second
# pipx (MacPorts : python314) passerait devant dans le PATH et réinstallerait scion.
if have pipx && have python3; then
  echo "  python3, pipx : déjà présents ($(command -v python3), $(command -v pipx)) — conservés"
else
  pkg_install python:python313 pipx:pipx
fi
SDK="$(xcrun --show-sdk-path 2>/dev/null || true)"
if [ -n "$SDK" ] && [ -f "$SDK/usr/include/expat.h" ]; then
  echo "expat : SDK ($SDK)"
else
  warn "expat.h absent du SDK : installation par $PKG (chemin $PKG_PREFIX à donner à CMake)."
  pkg_install expat:expat
fi

# --- scion : composition de l'arbre des sources (étape 0), même tag que Debian ---
SCION_REF="0.5.0.1"
log "scion ${SCION_REF}"
if have scion && scion version 2>/dev/null | grep -q "scion version: ${SCION_REF}\$"; then
  echo "  scion ${SCION_REF} : déjà installé ($(command -v scion))"
else
  pipx install --force "git+https://github.com/lepton-distribution/seed.scions.git@${SCION_REF}" \
    || warn "installation de scion échouée — bloquant pour l'arbre des sources."
fi
pipx ensurepath >/dev/null 2>&1 || true

# --- Outillage (métriques, documentation) --------------------------------------
log "Outillage (cloc, doxygen, graphviz)"
pkg_install cloc:cloc doxygen:doxygen
# graphviz : graphes de Doxygen seulement ; sans paquet binaire MacPorts sur Intel (darwin 24),
# compilé sur place avec une longue chaîne de dépendances : facultatif, comme coccinelle.
(pkg_install graphviz:graphviz) \
  || warn "graphviz non installé (facultatif : graphes de la documentation Doxygen)."
# coccinelle : outillage de transformation de la migration (terminée) ; facultatif ici.
(pkg_install coccinelle:coccinelle) \
  || warn "coccinelle non installé (facultatif : transformations de masse de la migration)."

# --- QEMU (socle mps2-an386 et mps2-an500, banc KAL) ----------------------------
log "QEMU (system-arm)"
# MacPorts : target_arm est une variante par défaut de qemu (2.12.6) ; demandée explicitement
# pour ne pas dépendre de ce défaut.
pkg_install "qemu:qemu +target_arm"
# Le test réseau QEMU (label net) exige un tap en espace de noms (unshare) : Linux seulement.

# --- Toolchain croisée ARM ------------------------------------------------------
# Arm GNU Toolchain 14.2.Rel1 (GCC 14.2.1, comme Debian 13), archive officielle d'Arm vérifiée
# par SHA-256. Pas le paquet du gestionnaire : il suit la dernière version, non épinglable.
ARM_VERSION="14.2.rel1"
ARM_NAME="arm-gnu-toolchain-${ARM_VERSION}-${ARM_HOST}-arm-none-eabi"
ARM_URL="https://developer.arm.com/-/media/Files/downloads/gnu/${ARM_VERSION}/binrel/${ARM_NAME}.tar.xz"
ARM_ROOT="$TOOLCHAIN_DIR/$ARM_NAME"
log "Toolchain ARM (${ARM_NAME})"
if [ -x "$ARM_ROOT/bin/arm-none-eabi-gcc" ]; then
  echo "  déjà installée : $ARM_ROOT"
else
  mkdir -p "$TOOLCHAIN_DIR"
  archive="$TOOLCHAIN_DIR/${ARM_NAME}.tar.xz"
  [ -f "$archive" ] || curl -fL --retry 3 -o "$archive" "$ARM_URL"
  got="$(shasum -a 256 "$archive" | awk '{print $1}')"
  if [ "$got" != "$ARM_SHA256" ]; then
    die "SHA-256 de $archive inattendu ($got) : archive supprimable, rien n'a été extrait."
  fi
  mkdir -p "$ARM_ROOT"
  tar -xJf "$archive" -C "$ARM_ROOT" --strip-components 1
  rm -f "$archive"
fi
export PATH="$ARM_ROOT/bin:$PATH"
# newlib-nano : même contrôle que install-debian.sh.
if arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 --specs=nano.specs \
     -print-file-name=libc_nano.a | grep -q '^/'; then
  echo "newlib-nano : OK"
else
  warn "libc_nano.a introuvable pour cortex-m4 hard-float (newlib-nano)."
fi

# --- Flash et débogage des cartes (étape 10) -----------------------------------
if [ "$DEBUG_TOOLS" = "1" ]; then
  log "Outils de débogage (OpenOCD ; gdb avec Python : arm-none-eabi-gdb du gestionnaire)"
  # +cmsis (hidapi) : sonde EDBG de la SAMD21 Xplained Pro, CMSIS-DAP v1 en HID seulement ;
  # sans elle, « unable to find a matching CMSIS-DAP device » (ST-LINK : libusb, déjà présent).
  pkg_install "open-ocd:openocd +ftdi +cmsis"
  # Le gdb de l'Arm GNU Toolchain est construit sans Python : les commandes lepton-stacks et
  # lepton-fault (debug/gdbinit-*), écrites en Python, n'y existent pas. gdb vient donc du
  # gestionnaire de paquets (étape 10).
  # Le port tire arm-none-eabi-gcc 16.x dans /opt/local/bin : la toolchain d'Arm (14.2.1) doit
  # rester devant dans le PATH. Il construit sa doc .info sans déclarer texinfo (makeinfo).
  pkg_install texinfo:texinfo "arm-none-eabi-gdb:arm-none-eabi-gdb +python313"
  GDB="$PKG_PREFIX/bin/arm-none-eabi-gdb"
  [ -x "$GDB" ] || die "$GDB introuvable après installation."
  "$GDB" -batch -ex "python print('ok')" 2>/dev/null | grep -qx ok \
    || die "$GDB sans Python (variante +python313 absente ?)."
  # Debian nomme gdb « gdb-multiarch » (tests/endurance_board.py l'appelle par ce nom) :
  # lien hors dépôt dans ~/.local/bin, à placer dans le PATH.
  mkdir -p "$HOME/.local/bin"
  ln -sfn "$GDB" "$HOME/.local/bin/gdb-multiarch"
  echo "  gdb-multiarch -> $GDB (~/.local/bin)"
fi

# --- Récapitulatif : versions à consigner dans MIGRATION-STATUS.md --------------
log "Versions installées (à consigner dans MIGRATION-STATUS.md)"
printf '  %-26s %s\n' "macOS" "$(sw_vers -productVersion) ($ARCH)"
printf '  %-26s %s\n' "paquets" "$PKG ($PKG_PREFIX)"
for c in cc cmake ninja python3 scion cloc spatch \
         arm-none-eabi-gcc arm-none-eabi-gdb gdb-multiarch qemu-system-arm doxygen openocd; do
  if command -v "$c" >/dev/null 2>&1; then
    # openocd écrit sa version sur stderr
    if [ "$c" = scion ]; then v="$(scion version 2>/dev/null | tail -1)"; else v="$("$c" --version 2>&1 | head -1)"; fi
    printf '  %-26s %s\n' "$c" "$v"
  else
    printf '  %-26s (absent)\n' "$c"
  fi
done
newlib="$(printf '#include <_newlib_version.h>\n_NEWLIB_VERSION\n' \
          | arm-none-eabi-gcc -E -P - 2>/dev/null | tail -1 || true)"
printf '  %-26s %s\n' "newlib (toolchain ARM)" "${newlib:-(non relevée)}"
for m in mps2-an386 mps2-an500; do
  if qemu-system-arm -machine help 2>/dev/null | grep -q "^$m "; then
    printf '  %-26s %s\n' "QEMU $m" "présente"
  else
    printf '  %-26s %s\n' "QEMU $m" "ABSENTE"
  fi
done

echo
echo "À ajouter au fichier de démarrage du shell (~/.zprofile), non fait par ce script :"
echo "  export PATH=\"$ARM_ROOT/bin:\$PATH\""
echo
echo "Reste à faire à la main : copier le paquet embOS-Classic V5.20.0.0 Cortex-M GCC dans"
echo "  <ROOTSTOCK>/third_party/embos/cortexm-gcc/5.20.0.0/  (doc/BUILDING.md §1)."
echo
echo "Terminé."
