/*
 * Lepton — pilotes de l'hôte Linux (noyau statique de mklepton). Licence : voir LICENSE (MPL 1.1).
 *
 * Interface entre les pilotes Lepton de l'hôte (compilés en freestanding avec les en-têtes Lepton)
 * et leur partie POSIX (host_posix.c, compilée avec les en-têtes de la glibc). Types C de base
 * uniquement : ce fichier est inclus des deux côtés.
 */
#ifndef _LEPTON_HOST_POSIX_H_
#define _LEPTON_HOST_POSIX_H_

/* Fichier image du disque hôte. Chemin relatif au répertoire courant du processus. */
int  host_file_open(const char* path);
void host_file_close(int fd);
long host_file_size(int fd);
int  host_file_resize(int fd, long size);
int  host_file_pread(int fd, char* buf, int size, long offset);
int  host_file_pwrite(int fd, const char* buf, int size, long offset);

/* Horloge : SOURCE_DATE_EPOCH si défini (sorties reproductibles), sinon l'heure de l'hôte.
 * Remplit sec, min, hour, mday, mon (0-11), year (depuis 1900), en UTC. Retourne 0 ou -1. */
int host_clock_now(int tm_fields[6]);

#endif
