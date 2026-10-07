#!/usr/bin/env bash
# mklepton_empreintes.sh — empreintes SHA-256 des sorties de mklepton (étape 8).
#
# Lance mklepton sur le mkconf de chaque carte (cmake/boards/*.cmake : LEPTON_BOARD_MKCONF,
# LEPTON_BOARD_MKCONF_TARGET) et sur le mkconf du test host.mklepton, puis écrit ou compare la
# liste des empreintes de tous les fichiers produits (kernel_mkconf.h, dev_mkconf.c,
# bin_mkconf.c, dev_dskimg.c/.h, .boot, .mount, image .fsflash.o).
# Usage : prouver que mklepton produit les mêmes octets quel que soit l'hôte (i386, x86_64,
# gcc, clang, Linux, macOS). SOURCE_DATE_EPOCH=0, comme cmake/mklepton.cmake.
#
#   mklepton_empreintes.sh --mklepton <exe> [--trunk <dir>] --out <fichier>    écrit la référence
#   mklepton_empreintes.sh --mklepton <exe> [--trunk <dir>] --check <fichier>  compare (code 1 si écart)
#
# --trunk : racine de l'arbre (répertoire de CMakeLists.txt) ; défaut : deux niveaux au-dessus.
# Aucune écriture dans le trunk : sorties dans un répertoire temporaire, supprimé à la fin.
# Compatible bash 3.2 (macOS) ; sha256sum ou, à défaut, shasum -a 256.
set -eu

here="$(cd "$(dirname "$0")" && pwd)"
trunk="$(cd "$here/../.." && pwd)"
mklepton="" out="" check=""
while [ $# -gt 0 ]; do
  case "$1" in
    --mklepton) mklepton="$2"; shift 2 ;;
    --trunk)    trunk="$(cd "$2" && pwd)"; shift 2 ;;
    --out)      out="$2"; shift 2 ;;
    --check)    check="$2"; shift 2 ;;
    *) echo "option inconnue : $1" >&2; exit 2 ;;
  esac
done
if [ -z "$mklepton" ] || { [ -z "$out" ] && [ -z "$check" ]; } || { [ -n "$out" ] && [ -n "$check" ]; }; then
  echo "usage : $0 --mklepton <exe> [--trunk <dir>] (--out <fichier> | --check <fichier>)" >&2
  exit 2
fi
[ -x "$mklepton" ] || { echo "mklepton introuvable ou non exécutable : $mklepton" >&2; exit 2; }
case "$mklepton" in /*) ;; *) mklepton="$PWD/$mklepton" ;; esac

if command -v sha256sum >/dev/null 2>&1; then sha="sha256sum"; else sha="shasum -a 256"; fi

# couples « mkconf cible », sans doublon (QEMU an386 et an500 partagent leur mkconf)
couples="sys/user/tauon-basic/etc/mkconf_tauon_basic_lwip_stm32f4-olimex-p407.xml cortexm_lepton"
for board in "$trunk"/cmake/boards/*.cmake; do
  xml="$(sed -n 's/^set(LEPTON_BOARD_MKCONF \(.*\))$/\1/p' "$board")"
  cible="$(sed -n 's/^set(LEPTON_BOARD_MKCONF_TARGET \(.*\))$/\1/p' "$board")"
  if [ -z "$xml" ] || [ -z "$cible" ]; then
    echo "mkconf ou cible illisible dans $board" >&2; exit 2
  fi
  couples="$couples
$xml $cible"
done
couples="$(printf '%s\n' "$couples" | sort -u)"

work="$(mktemp -d "${TMPDIR:-/tmp}/mklepton-empreintes.XXXXXX")"
trap 'rm -rf "$work"' EXIT

printf '%s\n' "$couples" | while read -r xml cible; do
  nom="$(basename "$xml" .xml)"
  mkdir -p "$work/out/$nom"
  if ! (cd "$work/out/$nom" && SOURCE_DATE_EPOCH=0 "$mklepton" -s "$trunk" -o "$work/out/$nom" \
          -t "$cible" "$xml" >"$work/$nom.log" 2>&1); then
    echo "échec de mklepton sur $xml :" >&2; tail -20 "$work/$nom.log" >&2; exit 1
  fi
done

# liste triée, chemins relatifs ; LC_ALL=C : même ordre sur tous les hôtes
(cd "$work/out" && find . -type f | LC_ALL=C sort | while read -r f; do $sha "$f"; done) \
  | sed 's/  \.\//  /' >"$work/empreintes"
nb="$(wc -l <"$work/empreintes" | tr -d ' ')"
[ "$nb" -gt 0 ] || { echo "aucune sortie produite" >&2; exit 1; }

if [ -n "$out" ]; then
  cp "$work/empreintes" "$out"
  echo "mklepton_empreintes : $nb fichiers, référence écrite dans $out"
  exit 0
fi
if diff "$check" "$work/empreintes" >"$work/diff"; then
  echo "mklepton_empreintes : $nb fichiers identiques à $check"
else
  echo "mklepton_empreintes : écart avec $check (< référence, > cet hôte)" >&2
  cat "$work/diff" >&2
  exit 1
fi
