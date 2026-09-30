/*
 * Lepton — noyau statique hôte : pas d'image cpufs embarquée (même forme que le dev_dskimg.h
 * généré par mklepton). L'image construite par mklepton est sur le disque hôte (hdc).
 */
#ifndef _DEV_DSKIMG_H
#define _DEV_DSKIMG_H

extern const unsigned char filecpu_memory[];
extern const unsigned long filecpu_memory_size;

#endif
