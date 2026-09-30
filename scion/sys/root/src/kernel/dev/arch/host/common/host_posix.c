/*
 * Lepton — pilotes de l'hôte Linux (noyau statique de mklepton). Licence : voir LICENSE (MPL 1.1).
 *
 * Partie POSIX des pilotes hôte : compilée avec les en-têtes de la glibc, sans aucun en-tête
 * Lepton (voir host_posix.h). Remplace les appels système i386 (int $0x80) des pilotes gnu32 gelés.
 */
#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "host_posix.h"

int host_file_open(const char* path){
   return open(path, O_RDWR | O_CREAT | O_CLOEXEC, 0644);
}

void host_file_close(int fd){
   if(fd >= 0)
      close(fd);
}

long host_file_size(int fd){
   struct stat st;
   if(fstat(fd, &st) < 0)
      return -1;
   return (long)st.st_size;
}

/* Taille exacte, contenu remis à zéro : même effet que HDSETSZ de dev_linux_fileflash. */
int host_file_resize(int fd, long size){
   if(ftruncate(fd, 0) < 0)
      return -1;
   return ftruncate(fd, (off_t)size);
}

int host_file_pread(int fd, char* buf, int size, long offset){
   ssize_t r;
   do {
      r = pread(fd, buf, (size_t)size, (off_t)offset);
   } while(r < 0 && errno == EINTR);
   return (int)r;
}

int host_file_pwrite(int fd, const char* buf, int size, long offset){
   ssize_t w;
   do {
      w = pwrite(fd, buf, (size_t)size, (off_t)offset);
   } while(w < 0 && errno == EINTR);
   return (int)w;
}

int host_clock_now(int tm_fields[6]){
   const char* epoch = getenv("SOURCE_DATE_EPOCH");
   time_t now;
   struct tm tm;

   if(epoch && *epoch) {
      char* end;
      long long v = strtoll(epoch, &end, 10);
      if(*end != '\0' || v < 0)
         return -1;
      now = (time_t)v;
   } else {
      now = time((time_t*)0);
   }
   if(!gmtime_r(&now, &tm))
      return -1;
   tm_fields[0] = tm.tm_sec;
   tm_fields[1] = tm.tm_min;
   tm_fields[2] = tm.tm_hour;
   tm_fields[3] = tm.tm_mday;
   tm_fields[4] = tm.tm_mon;
   tm_fields[5] = tm.tm_year;
   return 0;
}
