/* Déclarations minimales de la libc pour le noyau statique hôte (freestanding, -nostdinc).
 * Les définitions viennent de la glibc à l'édition de liens (mêmes prototypes, même ABI).
 * Ne pas y ajouter de type Lepton : ce répertoire remplace les en-têtes système. */
#ifndef _LEPTON_HOST_STRING_H
#define _LEPTON_HOST_STRING_H
#include <stddef.h>
void *memcpy(void *, const void *, size_t);
void *memmove(void *, const void *, size_t);
void *memset(void *, int, size_t);
int memcmp(const void *, const void *, size_t);
void *memchr(const void *, int, size_t);
size_t strlen(const char *);
char *strcpy(char *, const char *);
char *strncpy(char *, const char *, size_t);
char *strcat(char *, const char *);
char *strncat(char *, const char *, size_t);
int strcmp(const char *, const char *);
int strncmp(const char *, const char *, size_t);
char *strchr(const char *, int);
char *strrchr(const char *, int);
char *strstr(const char *, const char *);
char *strtok(char *, const char *);
char *strtok_r(char *, const char *, char **); /* newlib / glibc (POSIX) : FAT msdos */
size_t strspn(const char *, const char *);
size_t strcspn(const char *, const char *);
char *strpbrk(const char *, const char *);
#endif
