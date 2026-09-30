# Handoff étape 3 → 4

État au 2026-09-30 : 3a TERMINÉE (validée) ; 3b À VALIDER (palier 7 vert). **Étape 3 non close** :
tâche 1 d'ETAPE-3 non livrée (ci-dessous), prérequis de l'étape 4. Détail de 3a :
`handoff/etape-3a.md` ; journal des paliers : `validation-qemu-mps2-an386.md`.

## Réponses aux prérequis de 4
- Socle QEMU vert : OUI. `ci/run.sh` vert : host 5/5 ; presets `qemu-mps2-an386-embos` (hard,
  principal) et `-soft` : fumée, `net.ping_ftpd`, banc KAL (hard 14, soft 10). Paliers 1-7 verts.
- Banc KAL T0-T8 : OUI (M4 × embOS), plus variantes FPU T1F/T4F/T6F/T7F en hard-float.
- `compiler.h` (`__lepton_*`, GCC seul) : **NON** — absent ; seul `kernel/core/kernel_compiler.h`
  (branches `__ICCARM__`/`__GNUC__`). Tâche 1 d'ETAPE-3, non livrée en 3a et non signalée.
- `transform_iar.py` amorcé : **NON** — `tools/migration/transform_iar.py` n'existe pas.
- `audit-iar.md`, périmètre actif (étape 1) : OUI ; `audit_iar.py --summary` : actif 147 (inchangé).

## Décisions actées pendant 3 (MIGRATION-STATUS, 2026-09-30)
- 3a : soft-float puis hard-float principal (soft conservé en CI) ; frontière libc ; démarrage et
  RTOSInit Lepton ; `bin` = T9-T11 ; embOS SP ; verrou d'appels système en sémaphore ; T2/T8
  alignés ; T8 FPU = aucun contexte hérité ; dette de sécurité FPU/MPU.
- 3b : lwIP 2.0.1 (`LEPTON_NET_STACK`, `USE_LWIP`) ; test réseau par tap en espace de noms
  (`unshare`, sans sudo) ; `strdup`/`strerror` dans la libc Lepton (pas newlib).

## Artefacts produits (manifeste)
| Fichier | Contenu, quand le lire |
|---|---|
| `validation-qemu-mps2-an386.md` | paliers 1-7, défauts corrigés et leurs causes, hypothèses |
| `traces/palier4-appel-systeme.*` | trace gdb du premier appel système (modèle de trace) |
| `scion/cmake/components/kernel.cmake` | bibliothèques par composant, dont `lepton_net_lwip` |
| `scion/cmake/components/firmware.cmake` | mklepton de la carte, pseudo-binaires (fichier ou répertoire), tests smoke/net/kal |
| `scion/cmake/boards/qemu-mps2-an386.cmake` | BSP, pile réseau, paramètres du test réseau |
| `scion/sys/root/src/kernel/dev/arch/all/eth/dev_eth_lan9118/` | pilote LAN9118 (modèle pour un pilote Ethernet) |
| `scion/sys/root/src/kernel/dev/bsp/qemu_mps2_an386/` | adresses, IRQ, instances ttys0/1 et eth0 |
| `scion/sys/root/src/lib/libc/string/strerror.c` | table errno Lepton → message |
| `scion/tests/net_qemu.py`, `smoke_lsh.py`, `tests/kal/` | tests `net`, `smoke`, `kal` |
| `ci/run.sh`, `scripts/install-debian.sh` | non-régression ; paquets `iproute2`, `iputils-ping` |

## Écarts au plan et pièges découverts
- Tâche 1 (`compiler.h`, `transform_iar.py`) : voir ci-dessus ; la chaîne minimale a été rendue
  compilable par corrections ponctuelles (3a) et non par le script prévu.
- Pas de SVC avec embOS (appel système = événement de tâche) ; E3 corrigé (cadre FPU étendu).
- Branches `#if !defined(__GNUC__)` = anciennes branches de la **simulation Linux**, pas du GCC
  croisé : valeurs fausses sur cible (priorité lwIP 10 → `socket()` avant l'init de lwIP). En
  chercher d'autres à l'étape 4 (`grep -R "__GNUC__"` sur le périmètre actif).
- `ARG_LEN_MAX` = 64 : ligne de commande tronquée sans message (garder les `.init` courts).
- `lwip/errno.h` (`LWIP_PROVIDE_ERRNO`) redéfinit les `E*` (numérotation Linux) : dette.
- GCC 14 : déclarations implicites et types de pointeurs incompatibles = erreurs (ftpd).
- Hook `lepton_guard.py` : `sed -i` relatif, heredoc ou `>` sont refusés si le répertoire courant
  est dans le trunk ; lancer depuis le clone, scripts dans le scratchpad.
- Trunk : `grep -R` (liens) ; `pkill -x qemu-system-arm` (pas `-f`).

## Non transmis volontairement
- Diagnostics gdb détaillés (argv d'`ifconfig`, état de lwIP au démarrage) : résumés dans le
  journal ; scripts non conservés.
- Sources QEMU de référence (`lan9118.c`, `mps2.c`) : non versionnées (tag v10.0.0 public).
