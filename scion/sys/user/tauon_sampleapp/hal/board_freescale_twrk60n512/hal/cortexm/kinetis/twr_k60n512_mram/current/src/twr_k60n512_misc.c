//==========================================================================
//
//      twr_k60n512_misc.c
//
//      Cortex-M4 TWR-K60N512 EVAL HAL functions
//
//==========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2011 Free Software Foundation, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later
// version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License
// along with eCos; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
// As a special exception, if other files instantiate templates or use
// macros or inline functions from this file, or you compile this file
// and link it with other works to produce a work based on this file,
// this file does not by itself cause the resulting work to be covered by
// the GNU General Public License. However the source code for this file
// must still be made available in accordance with section (3) of the GNU
// General Public License v2.
//
// This exception does not invalidate any other reasons why a work based
// on this file might be covered by the GNU General Public License.
// -------------------------------------------
// ####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):      ilijak
// Contributor(s):
// Date:           2011-02-05
// Description:
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <pkgconf/hal_cortexm.h>
#include <pkgconf/hal_cortexm_kinetis.h>
#include <pkgconf/hal_cortexm_kinetis_twr_k60n512.h>
#ifdef CYGPKG_KERNEL
   #include <pkgconf/kernel.h>
#endif

#include <cyg/infra/diag.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_arch.h>           // HAL header
#include <cyg/hal/hal_intr.h>           // HAL header

static inline void hal_gpio_init(void);

//
void hal_system_init( void );
void hal_platform_init( void );
void hal_system_init_vectors(void);
void hal_platform_init_clock(void);
void hal_platform_invoke_constructors(void);

// VSRs in vectors.S
__externC void hal_default_exception_vsr( void );
__externC void hal_default_interrupt_vsr( void );
__externC void hal_default_svc_vsr( void );
__externC void hal_pendable_svc_vsr( void );
__externC void hal_switch_state_vsr( void );

// DATA and BSS locations
__externC cyg_uint32 __ram_data_start;
__externC cyg_uint32 __ram_data_end;
__externC cyg_uint32 __rom_data_start;
__externC cyg_uint32 __sram_data_start;
__externC cyg_uint32 __sram_data_end;
__externC cyg_uint32 __srom_data_start;
__externC cyg_uint32 __bss_start;
__externC cyg_uint32 __bss_end;

// HAL and eCos functions
__externC cyg_uint32 hal_get_cpu_clock(void);
__externC void hal_start_main_clock(void);
__externC void cyg_start( void );

//
__externC cyg_uint32 hal_cortexm_systick_clock;
__externC cyg_uint32 hal_kinetis_sysclk;
__externC cyg_uint32 hal_kinetis_busclk;

#ifdef CYGHWR_HAL_CORTEXM_KINETIS_RTC
__externC void hal_start_rtc_clock(void);
#endif
//==========================================================================
// System init
//
// This is run to set up the basic system, including GPIO setting,
// clock feeds, power supply, and memory initialization. This code
// runs before the DATA is copied from ROM and the BSS cleared, hence
// it cannot make use of static variables or data tables.

__externC void CYGOPT_HAL_KINETIS_MISC_FLASH_SECTION_ATTR
hal_system_init( void )
{
#if defined(CYG_HAL_STARTUP_ROM)
   hal_wdog_disable();
   //disable cache FMC_PFB1CR and FMC_PFB0CR
   //HAL_WRITE_UINT32(CYGHWR_HAL_KINETIS_FMC_PFB0CR, 0x30000019);
   //HAL_WRITE_UINT32(CYGHWR_HAL_KINETIS_FMC_PFB1CR, 0x30000019);
   HAL_WRITE_UINT32(CYGHWR_HAL_KINETIS_FMC_PFB0CR, 0x30000000);
   HAL_WRITE_UINT32(CYGHWR_HAL_KINETIS_FMC_PFB1CR, 0x30000000);

   __asm__ volatile ( "nop" );
   __asm__ volatile ( "nop" );
   __asm__ volatile ( "nop" );
   __asm__ volatile ( "nop" );
   __asm__ volatile ( "nop" );
   __asm__ volatile ( "nop" );
   __asm__ volatile ( "nop" );
   __asm__ volatile ( "nop" );

#endif
   hal_gpio_init();
   hal_system_init_vectors();

   // Note: For CYG_HAL_STARTUP_SRAM, the SRAM_L bank simulates ROM
   // Relocate data from ROM to RAM
   {
      register cyg_uint32 *ram_p, *rom_p;
      for( ram_p = &__ram_data_start, rom_p = &__rom_data_start;
           ram_p < &__ram_data_end;
           ram_p++, rom_p++ )
         *ram_p = *rom_p;
   }

   // Relocate data from ROM to SRAM
   {
      register cyg_uint32 *ram_p, *sram_p;
      for( ram_p = &__sram_data_start, sram_p = &__srom_data_start;
           ram_p < &__sram_data_end;
           ram_p++, sram_p++ )
         *ram_p = *sram_p;
   }
   // Clear BSS
   {
      register cyg_uint32 *p;
      for( p = &__bss_start; p < &__bss_end; p++ )
         *p = 0;
   }
   // Initialize interrupt vectors. Set the levels for all interrupts
   // to default values. Also set the default priorities of the
   // system handlers: all exceptions maximum priority except SVC and
   // PendSVC which are lowest priority.
   {
      register int i;

      HAL_WRITE_UINT32( CYGARC_REG_NVIC_BASE+CYGARC_REG_NVIC_SHPR0, 0x00000000 );
      HAL_WRITE_UINT32( CYGARC_REG_NVIC_BASE+CYGARC_REG_NVIC_SHPR1, 0xFF000000 );
      HAL_WRITE_UINT32( CYGARC_REG_NVIC_BASE+CYGARC_REG_NVIC_SHPR2, 0x00FF0000 );

      hal_interrupt_handlers[CYGNUM_HAL_INTERRUPT_SYS_TICK] = (CYG_ADDRESS)hal_default_isr;

      for( i = 1; i < CYGNUM_HAL_ISR_COUNT; i++ )
      {
         hal_interrupt_handlers[i] = (CYG_ADDRESS)hal_default_isr;
         HAL_WRITE_UINT8( CYGARC_REG_NVIC_BASE+CYGARC_REG_NVIC_PR(
                             i-CYGNUM_HAL_INTERRUPT_EXTERNAL), 0x80 );
      }
   }
   // Enable Usage, Bus and Mem fault handlers. Do this for ROM and
   // JTAG startups. For RAM startups, this will have already been
   // done by the ROM monitor.
   {
      CYG_ADDRESS base = CYGARC_REG_NVIC_BASE;
      cyg_uint32 shcsr;

      HAL_READ_UINT32( base+CYGARC_REG_NVIC_SHCSR, shcsr );
      shcsr |= CYGARC_REG_NVIC_SHCSR_USGFAULTENA;
      shcsr |= CYGARC_REG_NVIC_SHCSR_BUSFAULTENA;
      shcsr |= CYGARC_REG_NVIC_SHCSR_MEMFAULTENA;
      HAL_WRITE_UINT32( base+CYGARC_REG_NVIC_SHCSR, shcsr );
   }

   //
   hal_platform_init();
   hal_platform_invoke_constructors();

   //launch user application
   cyg_start();
   for(;; ) ;

}

//===========================================================================
// hal_gpio_init
//===========================================================================
// Just for testing
// Not really smart to enable all modules..
#ifndef CYGHWR_HAL_KINETIS_SIM_SCGC1_ALL_M
#define CYGHWR_HAL_KINETIS_SIM_SCGC1_ALL_M \
 (CYGHWR_HAL_KINETIS_SIM_SCGC1_UART4_M | CYGHWR_HAL_KINETIS_SIM_SCGC1_UART5_M)
#endif

#ifndef CYGHWR_HAL_KINETIS_SIM_SCGC2_ALL_M
#define CYGHWR_HAL_KINETIS_SIM_SCGC2_ALL_M         \
            (CYGHWR_HAL_KINETIS_SIM_SCGC2_ENET_M | \
             CYGHWR_HAL_KINETIS_SIM_SCGC2_DAC0_M | \
             CYGHWR_HAL_KINETIS_SIM_SCGC2_DAC1_M)
#endif

#ifndef CYGHWR_HAL_KINETIS_SIM_SCGC3_ALL_M
#define CYGHWR_HAL_KINETIS_SIM_SCGC3_ALL_M             \
            (CYGHWR_HAL_KINETIS_SIM_SCGC3_RNGB_M |     \
             CYGHWR_HAL_KINETIS_SIM_SCGC3_FLEXCAN1_M | \
             CYGHWR_HAL_KINETIS_SIM_SCGC3_SPI2_M |     \
             CYGHWR_HAL_KINETIS_SIM_SCGC3_SDHC_M |     \
             CYGHWR_HAL_KINETIS_SIM_SCGC3_FTM2_M |     \
             CYGHWR_HAL_KINETIS_SIM_SCGC3_ADC1_M)
#endif

#ifndef CYGHWR_HAL_KINETIS_SIM_SCGC4_ALL_M
#define CYGHWR_HAL_KINETIS_SIM_SCGC4_ALL_M \
 (CYGHWR_HAL_KINETIS_SIM_SCGC4_EWM_M |CYGHWR_HAL_KINETIS_SIM_SCGC4_CMT_M |    \
 CYGHWR_HAL_KINETIS_SIM_SCGC4_I2C0_M |CYGHWR_HAL_KINETIS_SIM_SCGC4_I2C1_M |   \
 CYGHWR_HAL_KINETIS_SIM_SCGC4_UART0_M | CYGHWR_HAL_KINETIS_SIM_SCGC4_UART1_M |\
 CYGHWR_HAL_KINETIS_SIM_SCGC4_UART2_M | CYGHWR_HAL_KINETIS_SIM_SCGC4_UART3_M |\
 CYGHWR_HAL_KINETIS_SIM_SCGC4_USBOTG_M | CYGHWR_HAL_KINETIS_SIM_SCGC4_CMP_M | \
 CYGHWR_HAL_KINETIS_SIM_SCGC4_VREF_M | CYGHWR_HAL_KINETIS_SIM_SCGC4_LLWU_M)
#endif

#ifndef CYGHWR_HAL_KINETIS_SIM_SCGC5_ALL_M
#define CYGHWR_HAL_KINETIS_SIM_SCGC5_ALL_M            \
            (CYGHWR_HAL_KINETIS_SIM_SCGC5_LPTIMER_M | \
             CYGHWR_HAL_KINETIS_SIM_SCGC5_REGFILE_M | \
             CYGHWR_HAL_KINETIS_SIM_SCGC5_TSI_M |     \
             CYGHWR_HAL_KINETIS_SIM_SCGC5_PORTA_M |   \
             CYGHWR_HAL_KINETIS_SIM_SCGC5_PORTB_M |   \
             CYGHWR_HAL_KINETIS_SIM_SCGC5_PORTC_M |   \
             CYGHWR_HAL_KINETIS_SIM_SCGC5_PORTD_M |   \
             CYGHWR_HAL_KINETIS_SIM_SCGC5_PORTE_M)
#endif

#ifndef CYGHWR_HAL_KINETIS_SIM_SCGC6_ALL_M
#define CYGHWR_HAL_KINETIS_SIM_SCGC6_ALL_M             \
            (CYGHWR_HAL_KINETIS_SIM_SCGC6_FTFL_M |     \
             CYGHWR_HAL_KINETIS_SIM_SCGC6_DMAMUX_M |   \
             CYGHWR_HAL_KINETIS_SIM_SCGC6_FLEXCAN0_M | \
             CYGHWR_HAL_KINETIS_SIM_SCGC6_SPI0_M |    \
             CYGHWR_HAL_KINETIS_SIM_SCGC6_SPI1_M |     \
             CYGHWR_HAL_KINETIS_SIM_SCGC6_I2S_M |      \
             CYGHWR_HAL_KINETIS_SIM_SCGC6_CRC_M |      \
             CYGHWR_HAL_KINETIS_SIM_SCGC6_USBDCD_M |   \
             CYGHWR_HAL_KINETIS_SIM_SCGC6_PDB_M |      \
             CYGHWR_HAL_KINETIS_SIM_SCGC6_PIT_M |      \
             CYGHWR_HAL_KINETIS_SIM_SCGC6_FTM0_M |     \
             CYGHWR_HAL_KINETIS_SIM_SCGC6_FTM1_M |     \
             CYGHWR_HAL_KINETIS_SIM_SCGC6_ADC0_M |     \
             CYGHWR_HAL_KINETIS_SIM_SCGC6_RTC_M)
#endif

#ifndef CYGHWR_HAL_KINETIS_SIM_SCGC7_ALL_M
#define CYGHWR_HAL_KINETIS_SIM_SCGC7_ALL_M            \
            (CYGHWR_HAL_KINETIS_SIM_SCGC7_FLEXBUS_M | \
             CYGHWR_HAL_KINETIS_SIM_SCGC7_DMA_M |     \
             CYGHWR_HAL_KINETIS_SIM_SCGC7_MPU_M)
#endif

static inline void CYGOPT_HAL_KINETIS_MISC_FLASH_SECTION_ATTR
hal_gpio_init(void)
{
   cyghwr_hal_kinetis_sim_t *sim_p = CYGHWR_HAL_KINETIS_SIM_P;
   cyghwr_hal_kinetis_mpu_t *mpu_p = CYGHWR_HAL_KINETIS_MPU_P;

   // Enable clocks on all ports.
   sim_p->scgc1 = CYGHWR_HAL_KINETIS_SIM_SCGC1_ALL_M;
   sim_p->scgc2 = CYGHWR_HAL_KINETIS_SIM_SCGC2_ALL_M;
   sim_p->scgc3 = CYGHWR_HAL_KINETIS_SIM_SCGC3_ALL_M;
   sim_p->scgc4 = CYGHWR_HAL_KINETIS_SIM_SCGC4_ALL_M;
   sim_p->scgc5 = CYGHWR_HAL_KINETIS_SIM_SCGC5_ALL_M;
   sim_p->scgc6 = CYGHWR_HAL_KINETIS_SIM_SCGC6_ALL_M;
   sim_p->scgc7 = CYGHWR_HAL_KINETIS_SIM_SCGC7_ALL_M;

   // Disable MPU
   mpu_p->cesr = 0;
}

//===========================================================================
// hal_system_init_vectors
//===========================================================================
__externC void hal_system_init_vectors(void) {
   register int i;

   //init vector table
   for(i = 2; i < 15; i++) {
      hal_vsr_table[i] = (CYG_ADDRESS)hal_default_exception_vsr;
   }

   hal_vsr_table[CYGNUM_HAL_VECTOR_SERVICE] = (CYG_ADDRESS)hal_default_svc_vsr;
   hal_vsr_table[CYGNUM_HAL_VECTOR_PENDSV] = (CYG_ADDRESS)hal_pendable_svc_vsr;

   for(i = CYGNUM_HAL_VECTOR_SYS_TICK;
       i < CYGNUM_HAL_VECTOR_SYS_TICK + CYGNUM_HAL_VSR_MAX;
       i++) {
      hal_vsr_table[i] = (CYG_ADDRESS)hal_default_interrupt_vsr;
   }

   // On M3 and M4 parts, the NVIC contains a vector table base register.
   // We program this to relocate the vector table base to the base of SRAM.
   HAL_WRITE_UINT32( CYGARC_REG_NVIC_BASE+CYGARC_REG_NVIC_VTOR,
                     CYGARC_REG_NVIC_VTOR_TBLOFF(0)|
                     CYGARC_REG_NVIC_VTOR_TBLBASE_SRAM );

   // Use SVC to switch our state to thread mode running on the PSP.
   // We don't need to do this for RAM startup since the ROM code
   // will have already done it.
   hal_vsr_table[CYGNUM_HAL_VECTOR_SERVICE] = (CYG_ADDRESS)hal_switch_state_vsr;
   __asm__ volatile ("swi 0");
   hal_vsr_table[CYGNUM_HAL_VECTOR_SERVICE] = (CYG_ADDRESS)hal_default_svc_vsr;
}

//===========================================================================
// hal_platform_init
//===========================================================================
__externC void hal_platform_init(void) {
   // init clock
   hal_platform_init_clock();

   // Start up the system clock
   HAL_CLOCK_INITIALIZE( CYGNUM_HAL_RTC_PERIOD );
}

//===========================================================================
// hal_platform_init_clock
//===========================================================================
__externC void hal_platform_init_clock(void) {
   cyghwr_hal_kinetis_sim_t *sim_p = CYGHWR_HAL_KINETIS_SIM_P;
#ifdef CYGHWR_HAL_CORTEXM_KINETIS_TRACE_CLKOUT
   cyghwr_hal_kinetis_port_t *port_p = CYGHWR_HAL_KINETIS_PORTA_P;
#endif

# ifdef CYGHWR_HAL_CORTEXM_KINETIS_RTC
   // Real Time Clock
   hal_start_rtc_clock();
# endif

#if defined(CYG_HAL_STARTUP_ROM)
   // Main clock - MCG
   hal_start_main_clock();
   hal_kinetis_sysclk=hal_get_cpu_clock();
#else
   hal_kinetis_sysclk=CYGNUM_HAL_CORTEXM_KINETIS_MCGOUT_FREQ;
#endif

   hal_kinetis_busclk=hal_kinetis_sysclk /
                       CYGHWR_HAL_CORTEXM_KINETIS_CLKDIV_PER_BUS;
   hal_cortexm_systick_clock=hal_kinetis_sysclk;

   // Trace clock
#ifdef CYGHWR_HAL_CORTEXM_KINETIS_TRACECLK_CORE
   sim_p->sopt2 |= CYGHWR_HAL_KINETIS_SIM_SOPT2_TRACECLKSEL_M;
#else
   sim_p->sopt2 &= ~CYGHWR_HAL_KINETIS_SIM_SOPT2_TRACECLKSEL_M;
#endif
#ifdef CYGHWR_HAL_CORTEXM_KINETIS_TRACE_CLKOUT
   port_p->pcr[6] = CYGHWR_HAL_KINETIS_PORT_PCR_MUX(0x7);
#endif
}

//===========================================================================
// hal_platform_invoke_constructors
//===========================================================================
typedef void (*pfunc)(void);

extern pfunc __init_array_start__[];
extern pfunc __init_array_end__[];
#define CONSTRUCTORS_START  (__init_array_start__[0])
#define CONSTRUCTORS_END    (__init_array_end__)
#define NEXT_CONSTRUCTOR(c) ((c)++)

void hal_platform_invoke_constructors(void) {
   pfunc *p = &CONSTRUCTORS_START;
   //skip <_GLOBAL__I.10100_diag_write_char+1>, dummy binary in flash
   NEXT_CONSTRUCTOR(p);
   for (; p != CONSTRUCTORS_END; NEXT_CONSTRUCTOR(p))
      (*p)();
}

//==========================================================================

#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS

   #include CYGHWR_MEMORY_LAYOUT_H

//--------------------------------------------------------------------------
// Accesses to areas not backed by real devices or memory can cause
// the CPU to hang.
//
// The following table defines the memory areas that GDB is allowed to
// touch. All others are disallowed.
// This table needs to be kept up to date with the set of memory areas
// that are available on the board.

static struct {
   CYG_ADDRESS start;                   // Region start address
   CYG_ADDRESS end;                     // End address (last byte)
} hal_data_access[] =
{
   { CYGMEM_REGION_ram,        CYGMEM_REGION_ram+CYGMEM_REGION_ram_SIZE-1      },       // Main RAM
   #ifdef CYGMEM_REGION_sram
   { CYGMEM_REGION_sram,       CYGMEM_REGION_sram+CYGMEM_REGION_sram_SIZE-1    },       // On-chip SRAM
   #endif
   #ifdef CYGMEM_REGION_flash
   { CYGMEM_REGION_flash,      CYGMEM_REGION_flash+CYGMEM_REGION_flash_SIZE-1  },       // On-chip flash
   #endif
   #ifdef CYGMEM_REGION_rom
   { CYGMEM_REGION_rom,        CYGMEM_REGION_rom+CYGMEM_REGION_rom_SIZE-1      },       // External flash
   #endif
   { 0xE0000000,               0x00000000-1                                    },       // Cortex-M peripherals
   { 0x40000000,               0x60000000-1                                    },       // Chip specific peripherals
};

__externC int cyg_hal_stub_permit_data_access( CYG_ADDRESS addr, cyg_uint32 count )
{
   int i;
   for( i = 0; i < sizeof(hal_data_access)/sizeof(hal_data_access[0]); i++ ) {
      if( (addr >= hal_data_access[i].start) &&
          (addr+count) <= hal_data_access[i].end)
         return true;
   }
   return false;
}

#endif // CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS

//==========================================================================

#ifdef CYGPKG_REDBOOT
   #include <redboot.h>
   #include CYGHWR_MEMORY_LAYOUT_H

//--------------------------------------------------------------------------
// Memory layout
//
// We report the on-chip SRAM and external SRAM.

void
cyg_plf_memory_segment(int seg, unsigned char **start, unsigned char **end)
{
   switch (seg) {
   case 0:
      *start = (unsigned char *)CYGMEM_REGION_ram;
      *end = (unsigned char *)(CYGMEM_REGION_ram + CYGMEM_REGION_ram_SIZE);
      break;
   #ifdef CYGMEM_REGION_sram
   case 1:
      *start = (unsigned char *)CYGMEM_REGION_sram;
      *end = (unsigned char *)(CYGMEM_REGION_sram + CYGMEM_REGION_sram_SIZE);
      break;
   #endif
   default:
      *start = *end = NO_MEMORY;
      break;
   }
} // cyg_plf_memory_segment()

#endif // CYGPKG_REDBOOT


//==========================================================================
// EOF twr_k60n512_misc.c
