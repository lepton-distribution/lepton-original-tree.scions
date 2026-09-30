/*
 * Banc KAL — semihosting ARM (appels 0x04 SYS_WRITE0, 0x15 SYS_GET_CMDLINE, 0x20
 * SYS_EXIT_EXTENDED ; instruction BKPT 0xAB en Thumb).
 */
#include <stdint.h>
#include "kal_test.h"

volatile int kal_test_failures = 0;

static int semihost(int op, void* arg){
   register int r0 __asm__("r0") = op;
   register void* r1 __asm__("r1") = arg;
   __asm__ volatile ("bkpt 0xAB" : "+r"(r0) : "r"(r1) : "memory");
   return r0;
}

void kal_test_puts(const char* s){
   semihost(0x04, (void*)s);
}

void kal_test_put_u32(const char* label, uint32_t v){
   static const char hex[] = "0123456789abcdef";
   char buf[11];
   int i;
   buf[0] = '0';
   buf[1] = 'x';
   for(i = 0; i < 8; i++)
      buf[2 + i] = hex[(v >> (28 - 4 * i)) & 0xF];
   buf[10] = 0;
   kal_test_puts(label);
   kal_test_puts(buf);
   kal_test_puts("\n");
}

int kal_test_cmdline(char* buf, int size){
   struct { char* buf; int len; } blk = { buf, size };
   if(semihost(0x15, &blk) != 0)
      return -1;
   return blk.len;
}

void kal_test_exit(int code){
   /* ADP_Stopped_ApplicationExit (0x20026) + code de sortie */
   uint32_t blk[2] = { 0x20026u, (uint32_t)code };
   for(;;)
      semihost(0x20, blk);
}
