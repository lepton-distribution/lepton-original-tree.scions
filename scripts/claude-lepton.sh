#!/usr/bin/env bash
# claude-lepton.sh — lance Claude Code à la racine du clone Lepton avec accès au trunk et au build.
#   scripts/claude-lepton.sh [arguments passés à claude]
# Le trunk et le répertoire de build sont hors du répertoire de lancement : ils sont ajoutés
# explicitement (--add-dir), sinon Claude Code ne peut pas y lire.
set -euo pipefail
here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
# shellcheck source=/dev/null
source "$here/lepton-env.sh" "$here" >/dev/null
mkdir -p "$LEPTON_BUILD"
cd "$LEPTON_CLONE"
# --add-dir accepte plusieurs répertoires : placé avant la consigne, il la prendrait pour un
# répertoire. Les arguments de l'utilisateur (consigne, options) passent donc en premier.
exec claude "$@" --add-dir "$LEPTON_TRUNK" "$LEPTON_BUILD"
