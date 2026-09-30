/* Déclarations minimales de la libc pour le noyau statique hôte (freestanding, -nostdinc).
 * Les définitions viennent de la glibc (libm) à l'édition de liens.
 * Ne pas y ajouter de type Lepton : ce répertoire remplace les en-têtes système. */
#ifndef _LEPTON_HOST_MATH_H
#define _LEPTON_HOST_MATH_H
double pow(double, double);
#endif
