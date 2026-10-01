/* Déclarations minimales de la libc (freestanding, -nostdinc) : printf et NULL ; printf pour des
 * traces du noyau (vfs.c : _vfs_ls). Implémentation : glibc (hôte) ou newlib (cible).
 * L'API stdio applicative est celle de Lepton (lib/libc/stdio/stdio.h).
 * Ne pas y ajouter de type Lepton : ce répertoire remplace les en-têtes système. */
#ifndef _LEPTON_LIBC_DECL_STDIO_H
#define _LEPTON_LIBC_DECL_STDIO_H
int printf(const char*, ...);
/* NULL : exigé de <stdio.h> par POSIX (le HAL ST n'inclut que <stdio.h>) */
#ifndef NULL
#define NULL ((void*)0)
#endif
#endif
