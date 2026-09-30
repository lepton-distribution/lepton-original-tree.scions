/* Déclarations minimales de la libc pour le noyau statique hôte (freestanding, -nostdinc).
 * Les définitions viennent de la glibc à l'édition de liens (mêmes prototypes, même ABI).
 * Ne pas y ajouter de type Lepton : ce répertoire remplace les en-têtes système. */
#ifndef _LEPTON_HOST_STDLIB_H
#define _LEPTON_HOST_STDLIB_H
#include <stddef.h>
typedef struct { long quot; long rem; } ldiv_t;
typedef struct { int quot; int rem; } div_t;
ldiv_t ldiv(long, long);
div_t div(int, int);
void *malloc(size_t);
void *calloc(size_t, size_t);
void *realloc(void *, size_t);
void free(void *);
int atoi(const char *);
long atol(const char *);
long strtol(const char *, char **, int);
unsigned long strtoul(const char *, char **, int);
int abs(int);
long labs(long);
void exit(int);
void abort(void);
void qsort(void *, size_t, size_t, int (*)(const void *, const void *));
int rand(void);
void srand(unsigned);
#endif
