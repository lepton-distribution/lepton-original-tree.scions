/*
The contents of this file are subject to the Mozilla Public License Version 1.1
(the "License"); you may not use this file except in compliance with the License.
You may obtain a copy of the License at http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS" basis,
WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the
specific language governing rights and limitations under the License.

The Original Code is Lepton.

Alternatively, the contents of this file may be used under the terms of the eCos GPL license
(the  [eCos GPL] License), in which case the provisions of [eCos GPL] License are applicable
instead of those above. If you wish to allow use of your version of this file only under the
terms of the [eCos GPL] License and not to allow others to use your version of this file under
the MPL, indicate your decision by deleting  the provisions above and replace
them with the notice and other provisions required by the [eCos GPL] License.
If you do not delete the provisions above, a recipient may use your version of this file under
either the MPL or the [eCos GPL] License."
*/

/*============================================
| Compiler Directive
==============================================*/
//tsterrno <ip> <port> : connect() TCP vers un port fermé ; affiche l'errno reçu, en numérique.
//Test du palier réseau (tests/net_qemu.py, étape 4 ; /usr/sbin/net/tsterrno) : l'errno vu par l'application doit être
//dans la numérotation Lepton (kernel/core/errno.h), pas dans celle de lwIP (Linux). La
//comparaison est faite par le test hôte, sur les valeurs lues dans kernel/core/errno.h : la
//valeur des macros E* vue ici dépend de l'ordre des inclusions (lwip/errno.h).

/*============================================
| Includes
==============================================*/
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "kernel/core/kernelconf.h"
#include "kernel/core/libstd.h"
#include "lib/libc/unistd.h"
#include "lib/libc/stdio/stdio.h"
#include "lib/libc/net/socket.h"

/*============================================
| Implementation
==============================================*/

/*--------------------------------------------
| Name:        tsterrno_ip
| Description: adresse IPv4 pointée (a.b.c.d) en ordre réseau ; 0 si invalide
----------------------------------------------*/
static uint32_t tsterrno_ip(const char* s){
   uint32_t ip=0;
   int n;
   for(n=0; n<4; n++){
      uint32_t v=0;
      int digits=0;
      while(*s>='0' && *s<='9'){
         v=v*10+(uint32_t)(*s++-'0');
         digits++;
      }
      if(!digits || v>255 || (n<3 && *s++!='.'))
         return 0;
      ip=(ip<<8)|v;
   }
   return *s ? 0 : htonl(ip);
}

/*--------------------------------------------
| Name:        tsterrno_main
----------------------------------------------*/
int tsterrno_main(int argc,char* argv[]){
   struct sockaddr_in addr;
   int fd;
   int r;
   //
   if(argc<3){
      printf("usage: tsterrno <ip> <port>\r\n");
      return 1;
   }
   memset(&addr,0,sizeof(addr));
   addr.sin_family=AF_INET;
   addr.sin_port=htons((uint16_t)atoi(argv[2]));
   addr.sin_addr.s_addr=tsterrno_ip(argv[1]);
   if(!addr.sin_addr.s_addr){
      printf("tsterrno: adresse invalide\r\n");
      return 1;
   }
   //
   if((fd=socket(AF_INET,SOCK_STREAM,0))<0){
      printf("tsterrno: socket=%d errno=%d\r\n",fd,errno);
      return 1;
   }
   errno=0;
   r=connect(fd,(struct sockaddr*)&addr,sizeof(addr));
   printf("tsterrno: connect=%d errno=%d\r\n",r,errno);
   close(fd);
   return 0;
}

/*============================================
| End of Source  : tsterrno.c
==============================================*/
