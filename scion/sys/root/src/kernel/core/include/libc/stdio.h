/* Déclarations minimales de la libc (freestanding, -nostdinc) : printf seul, utilisé pour des
 * traces du noyau (vfs.c : _vfs_ls). Implémentation : glibc (hôte) ou newlib (cible).
 * L'API stdio applicative est celle de Lepton (lib/libc/stdio/stdio.h).
 * Ne pas y ajouter de type Lepton : ce répertoire remplace les en-têtes système. */
#ifndef _LEPTON_LIBC_DECL_STDIO_H
#define _LEPTON_LIBC_DECL_STDIO_H
int printf(const char*, ...);
#endif
