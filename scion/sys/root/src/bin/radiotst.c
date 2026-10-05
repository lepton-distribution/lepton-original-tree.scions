/*
 * Lepton — radiotst : test d'émission et de réception de chaînes sur /dev/radio (étape 6,
 * NUCLEO-WL55JC1, radio Sub-GHz FSK 868 MHz). Licence : voir LICENSE (MPL 1.1).
 *
 * Usage (deux cartes, une commande par carte) :
 *   radiotst tx <n> [période_ms]   émet n messages « RT <i> <n> » (i = 1..n), un par période
 *   radiotst rx <n> [délai_s]      reçoit les messages RT ; arrêt au dernier ou après délai_s sans
 *                                  donnée ; compte reçus, perdus, hors séquence, doublons
 *   radiotst ping <n> [période_ms] émet « PI <i> », attend la réponse « PO <i> » (1 s au plus)
 *   radiotst pong <n> [délai_s]    répond « PO <i> » à chaque « PI <i> » reçu, n fois au plus
 * Dernière ligne : « radiotst <mode> : … » ; code de retour 0 si aucun message perdu.
 *
 * Pilote /dev/radio (portage IAR, inchangé) : chaque write émet une trame fixe de 64 octets
 * complétée de zéros ; la réception remplit un tampon circulaire de 255 octets. Les messages sont
 * donc délimités par les octets nuls (et fins de ligne), quel que soit le découpage des read.
 * Période par défaut 1 000 ms : environ 12 ms d'émission par trame à 50 kbit/s, rapport cyclique
 * de la sous-bande 868,0-868,6 MHz (1 %, ETSI EN 300 220) respecté en moyenne horaire tant que
 * le nombre de trames par heure reste inférieur à 3 000.
 */
#include <stdint.h>
#include <string.h>

#include "kernel/core/kernelconf.h"
#include "kernel/core/libstd.h"
#include "kernel/core/fcntl.h"
#include "kernel/core/time.h"
#include "kernel/core/select.h"

#include "lib/libc/unistd.h"
#include "lib/libc/stdio/stdio.h"

#define RADIOTST_DEV        "/dev/radio"
#define RADIOTST_FRAME_MAX  64          /* charge utile du pilote */
#define RADIOTST_SEQ_MAX    1000        /* n maximal (table des reçus) */
#define RADIOTST_PONG_MS    1000        /* attente d'une réponse PO */

typedef struct {
   int fd;
   char tok[RADIOTST_FRAME_MAX + 1];
   int len;
} radiotst_rx_t;

static unsigned char radiotst_seen[RADIOTST_SEQ_MAX + 1];

static int radiotst_atoi(const char* s){
   int v = 0;
   if(!s || !*s)
      return -1;
   for(; *s; s++) {
      if(*s < '0' || *s > '9')
         return -1;
      v = v * 10 + (*s - '0');
   }
   return v;
}

/* message suivant (« XX <i> [<n>] ») ; 1 si lu, 0 si rien pendant timeout_ms, -1 si erreur */
static int radiotst_next(radiotst_rx_t* rx, char* kind, int* seq, int* n, int timeout_ms){
   for(;;) {
      fd_set fds;
      struct timeval tv;
      char buf[32];
      int cb, i;
      FD_ZERO(&fds);
      FD_SET(rx->fd, &fds);
      tv.tv_sec = timeout_ms / 1000;
      tv.tv_usec = (timeout_ms % 1000) * 1000;
      if(select(rx->fd + 1, &fds, (fd_set*)0, (fd_set*)0, &tv) <= 0)
         return 0;
      if((cb = read(rx->fd, buf, sizeof(buf))) < 0)
         return -1;
      for(i = 0; i < cb; i++) {
         char c = buf[i];
         if(c != '\0' && c != '\r' && c != '\n') {
            if(rx->len < RADIOTST_FRAME_MAX)
               rx->tok[rx->len++] = c;
            continue;
         }
         if(!rx->len)
            continue;
         rx->tok[rx->len] = '\0';
         rx->len = 0;
         /* « XX i » ou « XX i n » */
         if(rx->tok[2] == ' ' && rx->tok[0] && rx->tok[1]) {
            char* p = rx->tok + 3;
            char* sp = strchr(p, ' ');
            if(sp)
               *sp = '\0';
            kind[0] = rx->tok[0];
            kind[1] = rx->tok[1];
            kind[2] = '\0';
            *seq = radiotst_atoi(p);
            *n = sp ? radiotst_atoi(sp + 1) : -1;
            if(*seq > 0)
               return 1;
         }
         printf("radiotst : message ignore [%s]\r\n", rx->tok);
      }
   }
}

static int radiotst_send(int fd, const char* kind, int seq, int n){
   char msg[RADIOTST_FRAME_MAX];
   int len = n > 0 ? sprintf(msg, "%s %d %d", kind, seq, n) : sprintf(msg, "%s %d", kind, seq);
   return write(fd, msg, len) == len ? 0 : -1;
}

static int radiotst_usage(void){
   printf("usage : radiotst tx|rx|ping|pong <n> [periode_ms|delai_s]\r\n");
   return 2;
}

int radiotst_main(int argc, char* argv[]){
   radiotst_rx_t rx;
   char kind[3];
   int n, opt, seq, nn, i, r;
   int recus = 0, desordre = 0, doublons = 0, dernier = 0;
   if(argc < 3 || (n = radiotst_atoi(argv[2])) <= 0 || n > RADIOTST_SEQ_MAX)
      return radiotst_usage();
   opt = argc > 3 ? radiotst_atoi(argv[3]) : -1;
   memset(&rx, 0, sizeof(rx));
   memset(radiotst_seen, 0, sizeof(radiotst_seen));

   if(!strcmp(argv[1], "tx")) {
      int periode = opt > 0 ? opt : 1000;
      if((rx.fd = open(RADIOTST_DEV, O_WRONLY, 0)) < 0)
         return 1;
      for(i = 1; i <= n; i++) {
         if(radiotst_send(rx.fd, "RT", i, n) < 0)
            break;
         if(i < n)
            usleep(periode * 1000);
      }
      close(rx.fd);
      printf("radiotst tx : emis=%d/%d\r\n", i - 1, n);
      return (i - 1 == n) ? 0 : 1;
   }

   if(!strcmp(argv[1], "rx")) {
      int delai = (opt > 0 ? opt : 10) * 1000;
      if((rx.fd = open(RADIOTST_DEV, O_RDONLY, 0)) < 0)
         return 1;
      printf("radiotst rx : pret\r\n");
      while((r = radiotst_next(&rx, kind, &seq, &nn, delai)) > 0) {
         if(strcmp(kind, "RT") || seq > n)
            continue;
         if(radiotst_seen[seq]) {
            doublons++;
         } else {
            radiotst_seen[seq] = 1;
            recus++;
            if(seq < dernier)
               desordre++;
            dernier = seq;
         }
         if(seq == n)
            break;
      }
      close(rx.fd);
      printf("radiotst rx : recus=%d/%d perdus=%d desordre=%d doublons=%d\r\n",
             recus, n, n - recus, desordre, doublons);
      return (recus == n) ? 0 : 1;
   }

   if(!strcmp(argv[1], "ping")) {
      int periode = opt > 0 ? opt : 1000;
      if((rx.fd = open(RADIOTST_DEV, O_RDWR, 0)) < 0)
         return 1;
      for(i = 1; i <= n; i++) {
         if(radiotst_send(rx.fd, "PI", i, 0) < 0)
            break;
         /* réponse PO i attendue ; réponses en retard d'un ping précédent : ignorées */
         while((r = radiotst_next(&rx, kind, &seq, &nn, RADIOTST_PONG_MS)) > 0) {
            if(!strcmp(kind, "PO") && seq == i) {
               recus++;
               break;
            }
         }
         if(i < n)
            usleep(periode * 1000);
      }
      close(rx.fd);
      printf("radiotst ping : reponses=%d/%d perdues=%d\r\n", recus, n, n - recus);
      return (recus == n) ? 0 : 1;
   }

   if(!strcmp(argv[1], "pong")) {
      int delai = (opt > 0 ? opt : 10) * 1000;
      if((rx.fd = open(RADIOTST_DEV, O_RDWR, 0)) < 0)
         return 1;
      printf("radiotst pong : pret\r\n");
      while(recus < n && (r = radiotst_next(&rx, kind, &seq, &nn, delai)) > 0) {
         if(strcmp(kind, "PI"))
            continue;
         if(radiotst_send(rx.fd, "PO", seq, 0) < 0)
            break;
         recus++;
      }
      close(rx.fd);
      printf("radiotst pong : reponses=%d/%d\r\n", recus, n);
      return (recus == n) ? 0 : 1;
   }
   return radiotst_usage();
}
