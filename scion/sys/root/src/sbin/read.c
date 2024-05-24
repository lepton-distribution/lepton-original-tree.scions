/*
The contents of this file are subject to the Mozilla Public License Version 1.1
(the "License"); you may not use this file except in compliance with the License.
You may obtain a copy of the License at http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS" basis,
WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the
specific language governing rights and limitations under the License.

The Original Code is Lepton.

The Initial Developer of the Original Code is Philippe Le Boulanger.
Portions created by Philippe Le Boulanger are Copyright (C) 2024 <lepton.phlb@gmail.com>.
All Rights Reserved.

*/



/*===========================================
Includes
=============================================*/
#include <stdint.h>

#include "kernel/core/system.h"
#include "kernel/core/signal.h"
#include "kernel/core/libstd.h"
#include "kernel/core/fcntl.h"
#include "kernel/core/stat.h"
#include "kernel/core/statvfs.h"
#include "kernel/core/time.h"
#include "kernel/core/wait.h"


#include "lib/libc/unistd.h"
#include "lib/libc/stdio/stdio.h"

#include <string.h>
/*===========================================
Global Declaration
=============================================*/
#define READ_BUFFER_MAX 255

/*===========================================
Implementation
=============================================*/
/*-------------------------------------------
| Name: main
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int read_main (int argc,char* argv[]){
   uint8_t read_buffer[READ_BUFFER_MAX];
   int cb_r=0;
   //
   while((cb_r=read(STDIN_FILENO,read_buffer,sizeof(read_buffer)))>0){
      int cb_w = 0;
      //fragmentation management
      while (cb_w < cb_r) {
         int cb=0;
         if ((cb = write(STDOUT_FILENO, read_buffer + cb_w, cb_r - cb_w)) < 0) {
            return 0;
         }
         //
         cb_w += cb;
      }
     
   }
   return 0;
}

/*===========================================
End of Source read.c
=============================================*/
