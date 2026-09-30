/*
 * Banc KAL (BANC-TEST-KAL-QEMU.md) — harnais bare-metal : assertions, rapport et terminaison par
 * semihosting (qemu-system-arm -semihosting-config enable=on,target=native,arg=<test>).
 * Code de retour QEMU = nombre d'échecs (0 : succès).
 */
#ifndef _KAL_TEST_H_
#define _KAL_TEST_H_

#include <stdint.h>

void kal_test_puts(const char* s);
void kal_test_put_u32(const char* label, uint32_t v);
int  kal_test_cmdline(char* buf, int size);      /* argument de la ligne de commande semihosting */
void kal_test_exit(int code) __attribute__((noreturn));

extern volatile int kal_test_failures;

#define TEST_ASSERT(cond, msg) do { \
      if(!(cond)) { \
         kal_test_puts("ÉCHEC : "); kal_test_puts(msg); kal_test_puts("\n"); \
         kal_test_failures++; \
      } \
   } while(0)

#endif
