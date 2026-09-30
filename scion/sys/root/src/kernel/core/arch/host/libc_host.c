/*
 * Lepton — noyau statique hôte : fonctions de la libc Lepton exigées par le noyau (sysctl.c,
 * dev_part.c, systime.c), fournies par la glibc de l'hôte.
 * Licence : voir LICENSE (MPL 1.1).
 *
 * lib/libc/stdio/printf.c entraîne toute la couche stdio de Lepton (FILE, appels système write,
 * malloc système) : hors de propos pour un noyau sans appel système. Même contrat que
 * lib/libc (printf.c : __sprintf ; ctype.c : __lepton_libc_isascii).
 */
#include <stdarg.h>

int vsprintf(char* s, const char* fmt, va_list ap); /* glibc */

int __sprintf(char* sp, const char* fmt, ...){
   va_list ap;
   int r;
   va_start(ap,fmt);
   r=vsprintf(sp,fmt,ap);
   va_end(ap);
   return r;
}

int __lepton_libc_isascii(int ch){
   return (ch & ~0x7f)==0;
}
