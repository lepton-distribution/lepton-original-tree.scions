#!/usr/bin/env bash
# lepton-env.sh — à sourcer : localise le rootstock scion et exporte les chemins de travail.
#   source scripts/lepton-env.sh
# Exporte : LEPTON_ROOTSTOCK (répertoire portant .scion.rootstock.signature),
#           LEPTON_TRUNK (arbre composé, répertoire portant .scion.grafted.list),
#           LEPTON_CLONE (clone git de lepton-original-tree.scions),
#           LEPTON_BUILD (répertoire de build, hors du trunk).
# Ces variables sont propres au plan de migration ; scion lui-même n'en lit aucune.

_lepton_find_rootstock() {
  local d="${1:-$PWD}"
  d="$(cd "$d" && pwd -P)"
  while [ "$d" != "/" ]; do
    [ -f "$d/.scion.rootstock.signature" ] && { printf '%s\n' "$d"; return 0; }
    d="$(dirname "$d")"
  done
  return 1
}

if ! LEPTON_ROOTSTOCK="$(_lepton_find_rootstock "${1:-$PWD}")"; then
  echo "lepton-env: aucun rootstock scion au-dessus de ${1:-$PWD} (étape 0 non faite ?)" >&2
  return 1 2>/dev/null || exit 1
fi

LEPTON_TRUNK=""
for d in "$LEPTON_ROOTSTOCK"/*/; do
  [ -f "$d.scion.grafted.list" ] && { LEPTON_TRUNK="${d%/}"; break; }
done
if [ -z "$LEPTON_TRUNK" ]; then
  echo "lepton-env: aucun trunk greffé dans $LEPTON_ROOTSTOCK (lancer scion graft)" >&2
  return 1 2>/dev/null || exit 1
fi

LEPTON_CLONE="$LEPTON_ROOTSTOCK/depots/lepton/original/master"
LEPTON_BUILD="$LEPTON_ROOTSTOCK/build"
export LEPTON_ROOTSTOCK LEPTON_TRUNK LEPTON_CLONE LEPTON_BUILD
echo "rootstock: $LEPTON_ROOTSTOCK"
echo "trunk    : $LEPTON_TRUNK"
echo "clone    : $LEPTON_CLONE ($(git -C "$LEPTON_CLONE" branch --show-current 2>/dev/null))"
echo "build    : $LEPTON_BUILD"
