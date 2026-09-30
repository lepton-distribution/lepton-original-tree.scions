#!/usr/bin/env bash
# lib_unresolved.sh — symboles non résolus d'un ensemble de bibliothèques statiques (étape 2).
# Usage : tools/migration/lib_unresolved.sh <répertoire de build> [motif d'exclusion des .a]
# Affiche les symboles référencés par une bibliothèque et définis par aucune : ce qui doit venir
# de la libc de l'hôte, d'un backend, ou manque. Aucune écriture hors d'un fichier temporaire.
set -euo pipefail
dir=${1:?répertoire de build}
exclude=${2:-^$}
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
mapfile -t libs < <(find "$dir" -maxdepth 1 -name '*.a' | grep -Ev "$exclude" | sort)
nm -g --defined-only "${libs[@]}" 2>/dev/null | awk 'NF==3{print $3}' | sort -u > "$tmp/def"
nm -u "${libs[@]}" 2>/dev/null | awk 'NF>=2{print $NF}' | sort -u > "$tmp/und"
comm -23 "$tmp/und" "$tmp/def"
