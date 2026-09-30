# Sorties de référence mklepton — voie de repli

Généré par `tools/migration/mklepton_ref_manifest.py`. Décision utilisateur du 2026-09-30 : l'oracle binaire `mklepton_gnu` n'est pas exécutable (`libkernel.so` i386 absente, non reconstructible) ; la copie historique hors arbre n'est pas utilisée. La référence est le jeu de sorties déjà versionnées ci-dessous (non copiées : blob git au commit courant).

| Fichier (relatif au trunk) | Octets | sha256 | Blob git | mkconf d'origine |
|---|---|---|---|---|
| `sys/root/src/kernel/core/arch/win32/bin_mkconf.c` | 4195 | `2ead2e180d176679…` | `cf25066aa051` | `—` |
| `sys/root/src/kernel/core/arch/win32/dev_dskimg.c` | 96754 | `8234aab7539a6cf6…` | `7e6f45d77804` | `—` |
| `sys/root/src/kernel/core/arch/win32/dev_dskimg.h` | 595 | `bc0e1a13ad8c71fe…` | `bf9a3b24da93` | `—` |
| `sys/root/src/kernel/core/arch/win32/dev_mkconf.c` | 2148 | `7000961a8def4089…` | `e0b8dac75f52` | `—` |
| `sys/root/src/kernel/core/arch/win32/kernel_mkconf.h` | 940 | `3783efe574218432…` | `1e20c2ac1871` | `c:/tauon/sys/user/tauon-nuodio/etc/mkconf_tauon_nuodio_stm32f429_hybrid_tube.xml` |
| `sys/user/tauon-basic/etc/.boot` | 53 | `0a7a75f0f3560468…` | `d6e1c8f9d765` | `—` |
| `sys/user/tauon-basic/etc/.mount` | 21 | `998de1c6bf43d954…` | `eba5ac5e070f` | `—` |
| `sys/user/tauon_sampleapp/etc/.boot` | 60 | `46c48da7cc682b15…` | `59e834d9b3df` | `—` |
| `sys/user/tauon_sampleapp/etc/.mount` | 42 | `94359442c289c7af…` | `55138dd36f2c` | `—` |

## Usage à l'étape 2

- Format du C généré (`kernel_mkconf.h`, `dev_mkconf.c`, `bin_mkconf.c`, `dev_dskimg.[ch]`) : comparaison structurelle avec la sortie du mklepton natif (mêmes macros, tables `dev_lst`, `_bin_lst`, tableau `filecpu_memory[]`), chemins et horodatage exclus.
- `dev_dskimg.c` (win32) : image UFS au format MSVC `pack(1)` (nœud de 18 o contre 24 o sous GCC, `noyau-statique.md`) ; utilisable pour décoder la structure (superbloc `ufs 1.5`), pas pour une comparaison octet à octet. Son mkconf d'origine (`tauon-nuodio`) est absent de l'arbre.
- `.boot` / `.mount` : texte, comparaison directe.
- Image UFS produite par le mklepton natif : validée par exécution (montage et lecture sous QEMU, étape 3), pas par comparaison binaire.
