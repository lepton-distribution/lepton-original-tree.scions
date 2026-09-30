/* Déclarations minimales de la libc pour le noyau statique hôte (freestanding, -nostdinc).
 * Les définitions viennent de la glibc à l'édition de liens (mêmes prototypes, même ABI).
 * Ne pas y ajouter de type Lepton : ce répertoire remplace les en-têtes système. */
#ifndef _LEPTON_HOST_CTYPE_H
#define _LEPTON_HOST_CTYPE_H
int isalnum(int); int isalpha(int); int isdigit(int); int isspace(int); int isupper(int);
int islower(int); int isxdigit(int); int isprint(int); int toupper(int); int tolower(int);
#endif
