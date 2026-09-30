#!/bin/bash
# mklepton_oracle.sh — jeu de sorties de référence de mklepton (oracle du portage natif, étape 2).
#
# Exécute le binaire historique tools/bin/mklepton_gnu (ELF i386) sur chaque mkconf*.xml du trunk,
# dans un bac à sable : $LEPTON_BUILD/etape-1/mklepton-ref/<mkconf>/<cible>/
#   home/  : HOME du processus (remplace $(HOME) des mkconf)
#   work/  : répertoire courant (mklepton y écrit .fsflash.o, .fsrom.o, .boot, .mount par défaut)
#   out/   : cible de TOUS les dest_path de sortie (<mklepton>, <arch>, <boot>, <mount>)
#   mkconf.xml (réécrit), cmd.txt, stdout.log, rc.txt, sha256sums.txt, outputs.tar
# Rien n'est écrit dans le trunk, dans le clone ni dans le vrai $HOME (garde-fou vérifié avant
# toute exécution). Les chemins sources (src_file/src_path) sont remappés en LECTURE vers le trunk.
#
# Prérequis d'exécution (hors --dry-run) :
#   - /lib/ld-linux.so.2 (libc6:i386) et libexpat.so.1 i386 (apt install libexpat1:i386) ;
#   - libkernel.so (i386, bibliothèque PARTAGÉE) : absente du trunk, du clone et du paquet de
#     migration ; fournir le répertoire d'une copie validée par --libkernel-dir (RPATH du binaire :
#     /home/sqzwork/tauon/..., inexistant). Sans elle, l'oracle binaire est infaisable
#     (doc/migration/mklepton.md §5 : voies de repli).
#
# Usage : mklepton_oracle.sh [--dry-run] [--libkernel-dir DIR] [--targets t1,t2] [--only motif]
#                            [--timeout s] [--out DIR]
#   --targets : cibles XML à produire (défaut : toutes celles déclarées dans chaque mkconf)
#   --only    : ne traiter que les mkconf dont le chemin contient ce motif
# Non-déterminisme connu : cmtime des inœuds = horloge de l'hôte (dev_linux_rtc, syscall direct) ;
# la comparaison avec le portage doit masquer ce champ (offset 12, 4 octets, nœud UFS 1.4/1.5).
set -euo pipefail

SELF_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CLONE_ROOT="$(cd "$SELF_DIR/../.." && pwd)"
if [ -z "${LEPTON_TRUNK:-}" ] || [ -z "${LEPTON_BUILD:-}" ]; then
  # shellcheck disable=SC1091
  source "$CLONE_ROOT/scripts/lepton-env.sh" "$CLONE_ROOT" >/dev/null
fi
TRUNK="$(readlink -f "$LEPTON_TRUNK")"
CLONE="$(readlink -f "${LEPTON_CLONE:-$CLONE_ROOT}")"
REAL_HOME="$(readlink -f "$HOME")"

DRY=0; LIBK=""; TARGETS=""; ONLY=""; TMO=120
OUT="$LEPTON_BUILD/etape-1/mklepton-ref"
while [ $# -gt 0 ]; do
  case "$1" in
    --dry-run) DRY=1 ;;
    --libkernel-dir) LIBK="$2"; shift ;;
    --targets) TARGETS="$2"; shift ;;
    --only) ONLY="$2"; shift ;;
    --timeout) TMO="$2"; shift ;;
    --out) OUT="$2"; shift ;;
    -h|--help) sed -n '2,25p' "$0"; exit 0 ;;
    *) echo "option inconnue : $1" >&2; exit 2 ;;
  esac
  shift
done

die() { echo "mklepton_oracle: $*" >&2; exit 1; }

# --- garde-fou : un chemin résolu ne doit être ni sous le trunk, ni sous le clone, ni le vrai HOME
is_forbidden() {
  local p; p="$(readlink -m "$1")"
  case "$p/" in
    "$TRUNK"/*|"$CLONE"/*) return 0 ;;
  esac
  # le vrai HOME lui-même (et $HOME/tauon) est interdit, sauf s'il contient le bac à sable
  case "$p/" in
    "$REAL_HOME/tauon"/*) return 0 ;;
  esac
  [ "$p" = "$REAL_HOME" ] && return 0
  return 1
}
OUT="$(readlink -m "$OUT")"
is_forbidden "$OUT" && die "répertoire de sortie interdit : $OUT"

BIN="$TRUNK/tools/bin/mklepton_gnu"
[ -f "$BIN" ] || die "binaire absent : $BIN"

# --- prérequis de chargement (bloquants hors --dry-run)
PREREQ_OK=1
if [ ! -e /lib/ld-linux.so.2 ]; then
  echo "prérequis manquant : /lib/ld-linux.so.2 (paquet libc6:i386 ; dpkg --add-architecture i386)" >&2
  PREREQ_OK=0
fi
if ! ls /usr/lib/i386-linux-gnu/libexpat.so.1 /lib/i386-linux-gnu/libexpat.so.1 >/dev/null 2>&1; then
  echo "prérequis manquant : libexpat.so.1 i386 (paquet libexpat1:i386)" >&2
  PREREQ_OK=0
fi
if [ -n "$LIBK" ]; then
  LIBK="$(readlink -f "$LIBK")"
  [ -f "$LIBK/libkernel.so" ] || { echo "libkernel.so absent de $LIBK" >&2; PREREQ_OK=0; }
else
  echo "prérequis manquant : libkernel.so (i386) — option --libkernel-dir DIR" >&2
  PREREQ_OK=0
fi
if [ -e /lib/ld-linux.so.2 ]; then
  # diagnostic de chargement (ldd n'exécute pas le programme pour un ELF dynamique standard)
  LD_LIBRARY_PATH="${LIBK}" ldd "$BIN" 2>&1 | grep -E 'not found|introuvable' >&2 && PREREQ_OK=0 || true
fi
if [ "$DRY" -eq 0 ] && [ "$PREREQ_OK" -eq 0 ]; then
  die "prérequis absents : arrêt (utiliser --dry-run pour préparer les bacs à sable sans exécuter)"
fi

REWRITE="$SELF_DIR/mklepton_oracle_rewrite.py"
[ -f "$REWRITE" ] || die "script de réécriture absent : $REWRITE"

mkdir -p "$OUT"
SUMMARY="$OUT/summary.csv"
echo "mkconf;cible;rc;nb_sorties;dossier" > "$SUMMARY"
{
  echo "date_utc=$(date -u +%Y-%m-%dT%H:%M:%SZ)"
  echo "binaire=$BIN"
  echo "binaire_sha256=$(sha256sum "$BIN" | cut -d' ' -f1)"
  echo "libkernel_dir=${LIBK:-<absent>}"
  [ -n "$LIBK" ] && [ -f "$LIBK/libkernel.so" ] && echo "libkernel_sha256=$(sha256sum "$LIBK/libkernel.so" | cut -d' ' -f1)"
  echo "dry_run=$DRY"
} > "$OUT/run-info.txt"

mapfile -t MKCONFS < <(cd "$TRUNK" && find -L . -iname 'mkconf*.xml' -type f | sed 's|^\./||' | sort)
[ "${#MKCONFS[@]}" -gt 0 ] || die "aucun mkconf*.xml dans $TRUNK"

for rel in "${MKCONFS[@]}"; do
  [ -n "$ONLY" ] && [[ "$rel" != *"$ONLY"* ]] && continue
  src="$TRUNK/$rel"
  stem="$(echo "$rel" | sed 's|/|__|g; s|\.xml$||')"
  if [ -n "$TARGETS" ]; then
    IFS=',' read -r -a tlist <<< "$TARGETS"
  else
    mapfile -t tlist < <(python3 "$REWRITE" --list-targets "$src" | grep -v '^$' || true)
    [ "${#tlist[@]}" -gt 0 ] || tlist=("_none_")
  fi
  for tgt in "${tlist[@]}"; do
    box="$OUT/$stem/$tgt"
    is_forbidden "$box" && die "bac à sable interdit : $box"
    rm -rf "$box"; mkdir -p "$box/home" "$box/work" "$box/out"
    # réécriture : $(HOME)->box/home ; dest_path de sortie->box/out ; sources->trunk (lecture)
    python3 "$REWRITE" --in "$src" --out-xml "$box/mkconf.xml" --home "$box/home" \
        --dest "$box/out" --trunk "$TRUNK" --report "$box/rewrite.txt"
    # garde-fou : tous les chemins de sortie résolus
    while IFS= read -r p; do
      is_forbidden "$p" && die "chemin de sortie interdit après réécriture ($rel/$tgt) : $p"
    done < <(python3 "$REWRITE" --list-outputs "$box/mkconf.xml")
    for p in "$box/work" "$box/home" "$box/out"; do
      is_forbidden "$p" && die "chemin interdit : $p"
    done
    args=()
    [ "$tgt" != "_none_" ] && args=(-t "$tgt")
    cmd=(env -i "PATH=/usr/bin:/bin" "HOME=$box/home" "LD_LIBRARY_PATH=${LIBK}"
         timeout "$TMO" "$BIN" "${args[@]}" "$box/mkconf.xml")
    printf '%q ' "cd" "$box/work" >  "$box/cmd.txt"; echo "&&" >> "$box/cmd.txt"
    printf '%q ' "${cmd[@]}"      >> "$box/cmd.txt"; echo    >> "$box/cmd.txt"
    if [ "$DRY" -eq 1 ]; then
      echo "dry-run" > "$box/rc.txt"
      echo "$rel;$tgt;dry-run;0;$box" >> "$SUMMARY"
      echo "[dry-run] $rel [$tgt] -> $box"
      continue
    fi
    set +e
    (cd "$box/work" && "${cmd[@]}") > "$box/stdout.log" 2>&1
    rc=$?
    set -e
    echo "$rc" > "$box/rc.txt"
    # archivage : toutes les sorties (work/ et out/), empreintes
    (cd "$box" && find work out -type f -print0 | sort -z | xargs -0 -r sha256sum) > "$box/sha256sums.txt"
    (cd "$box" && tar --sort=name -cf outputs.tar work out)
    n=$(wc -l < "$box/sha256sums.txt")
    echo "$rel;$tgt;$rc;$n;$box" >> "$SUMMARY"
    echo "$rel [$tgt] rc=$rc sorties=$n"
  done
done
echo "résumé : $SUMMARY"
