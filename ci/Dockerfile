# ci/Dockerfile — environnement de référence de la CI Lepton (étape 6).
#
# Image construite sur scripts/install-debian.sh (mêmes paquets que l'hôte Debian natif, sans
# OpenOCD ni règles udev : pas de sonde dans le conteneur). Les sources ne sont pas copiées :
# le rootstock scion est monté entier (les liens du trunk sont relatifs à lui), ainsi que le
# paquet embOS s'il est hors du rootstock (LEPTON_EMBOS_ROOT, licence SFL, hors git).
#
# Construction (depuis la racine du clone) :
#   podman build -t lepton-ci -f ci/Dockerfile scripts
# Exécution (rootstock ~/lepton, monté au même chemin pour garder les chemins du cache CMake) :
#   podman run --rm --security-opt seccomp=unconfined --userns=keep-id \
#     -v ~/lepton:/home/lepton-user/lepton -w /home/lepton-user/lepton/depots/lepton/original/master \
#     lepton-ci ci/run.sh
#
# HYPOTHÈSE À VALIDER : image jamais construite ni exécutée (pas de moteur de conteneur sur
# l'hôte, décision utilisateur 2026-10-05) ; la CI est validée en natif par ci/run.sh.
# HYPOTHÈSE À VALIDER : les tests réseau (tests/net_qemu.py : unshare --user --net, tap) exigent
# que le conteneur autorise les espaces de noms utilisateur (seccomp=unconfined ci-dessus, ou
# --cap-add=SYS_ADMIN,NET_ADMIN --device /dev/net/tun selon le moteur).

# Debian 13 (trixie), point 13.7 de l'hôte de validation (versions épinglées :
# doc/migration/MIGRATION-STATUS.md). HYPOTHÈSE À VALIDER : étiquette non vérifiée sur le
# registre ; épingler ensuite par condensat (debian:13.7-slim@sha256:…).
FROM debian:13.7-slim

# scion installé par pipx hors de $HOME : commande accessible à tout utilisateur (--userns=keep-id)
ENV PIPX_HOME=/opt/pipx \
    PIPX_BIN_DIR=/usr/local/bin \
    LANG=C.UTF-8 \
    PYTHONDONTWRITEBYTECODE=1

COPY install-debian.sh /tmp/install-debian.sh
RUN /tmp/install-debian.sh && rm /tmp/install-debian.sh

CMD ["ci/run.sh"]
