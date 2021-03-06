#ifndef CYGONCE_HAL_PLF_IO_H
#define CYGONCE_HAL_PLF_IO_H
//=============================================================================
//
//      plf_io.h
//
//      Platform specific registers
//
//=============================================================================
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   ilijak
// Date:        2011-02-05
// Purpose:     TWR-K60N512 platform specific registers
// Description:
// Usage:       #include <cyg/hal/plf_io.h>
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>
#include <pkgconf/hal_cortexm_kinetis_twr_k60n512.h>


// UART PINs

#ifndef CYGHWR_HAL_FREESCALE_UART3_PIN_RX
# define CYGHWR_HAL_FREESCALE_UART3_PIN_RX CYGHWR_HAL_KINETIS_PIN(C, 16, 3, 0)
# define CYGHWR_HAL_FREESCALE_UART3_PIN_TX CYGHWR_HAL_KINETIS_PIN(C, 17, 3, 0)
# define CYGHWR_HAL_FREESCALE_UART3_PIN_RTS CYGHWR_HAL_KINETIS_PORT_PIN_NONE
# define CYGHWR_HAL_FREESCALE_UART3_PIN_CTS CYGHWR_HAL_KINETIS_PORT_PIN_NONE

# define CYGHWR_IO_FREESCALE_UART3_PIN_RX CYGHWR_HAL_FREESCALE_UART3_PIN_RX
# define CYGHWR_IO_FREESCALE_UART3_PIN_TX CYGHWR_HAL_FREESCALE_UART3_PIN_TX
# define CYGHWR_IO_FREESCALE_UART3_PIN_RTS CYGHWR_HAL_FREESCALE_UART3_PIN_RTS
# define CYGHWR_IO_FREESCALE_UART3_PIN_CTS CYGHWR_HAL_FREESCALE_UART3_PIN_CTS
#endif

// ENET PINs

// MDIO
#define CYGHWR_IO_FREESCALE_ENET0_PIN_RMII0_MDIO    CYGHWR_HAL_KINETIS_PIN(B, 0, 4, 0)
#define CYGHWR_IO_FREESCALE_ENET0_PIN_RMII0_MDC     CYGHWR_HAL_KINETIS_PIN(B, 1, 4, 0)
// Both RMII and MII interface
#define CYGHWR_IO_FREESCALE_ENET0_PIN_RMII0_RXER    CYGHWR_HAL_KINETIS_PIN(A, 5, 4, \
                                                    CYGHWR_HAL_KINETIS_PORT_PCR_PE_M)
#define CYGHWR_IO_FREESCALE_ENET0_PIN_RMII0_RXD1    CYGHWR_HAL_KINETIS_PIN(A, 12, 4, 0)
#define CYGHWR_IO_FREESCALE_ENET0_PIN_RMII0_RXD0    CYGHWR_HAL_KINETIS_PIN(A, 13, 4, 0)
#define CYGHWR_IO_FREESCALE_ENET0_PIN_RMII0_TXEN    CYGHWR_HAL_KINETIS_PIN(A, 15, 4, 0)
#define CYGHWR_IO_FREESCALE_ENET0_PIN_RMII0_TXD0    CYGHWR_HAL_KINETIS_PIN(A, 16, 4, 0)
#define CYGHWR_IO_FREESCALE_ENET0_PIN_RMII0_TXD1    CYGHWR_HAL_KINETIS_PIN(A, 17, 4, 0)
// RMII interface only
#define CYGHWR_IO_FREESCALE_ENET0_PIN_RMII0_CRS_DV  CYGHWR_HAL_KINETIS_PIN(A, 14, 4, 0)
// MII interface only
#define CYGHWR_IO_FREESCALE_ENET0_PIN_MII0_RXD3     CYGHWR_HAL_KINETIS_PIN(A, 9, 4, 0)
#define CYGHWR_IO_FREESCALE_ENET0_PIN_MII0_RXD2     CYGHWR_HAL_KINETIS_PIN(A, 10, 4, 0)
#define CYGHWR_IO_FREESCALE_ENET0_PIN_MII0_RXCLK    CYGHWR_HAL_KINETIS_PIN(A, 11, 4, 0)
#define CYGHWR_IO_FREESCALE_ENET0_PIN_MII0_TXD2     CYGHWR_HAL_KINETIS_PIN(A, 24, 4, 0)
#define CYGHWR_IO_FREESCALE_ENET0_PIN_MII0_TXCLK    CYGHWR_HAL_KINETIS_PIN(A, 25, 4, 0)
#define CYGHWR_IO_FREESCALE_ENET0_PIN_MII0_TXD3     CYGHWR_HAL_KINETIS_PIN(A, 26, 4, 0)
#define CYGHWR_IO_FREESCALE_ENET0_PIN_MII0_CRS      CYGHWR_HAL_KINETIS_PIN(A, 27, 4, 0)
#define CYGHWR_IO_FREESCALE_ENET0_PIN_MIIO_TXER     CYGHWR_HAL_KINETIS_PIN(A, 28, 4, 0)
#define CYGHWR_IO_FREESCALE_ENET0_PIN_MII0_COL      CYGHWR_HAL_KINETIS_PIN(A, 29, 4, 0)
// IEEE 1588 timers
#define CYGHWR_IO_FREESCALE_ENET0_PIN_1588_CLKIN    CYGHWR_HAL_KINETIS_PIN(E, 26, 4, 0)

#if defined(CYGHWR_HAL_FREESCALE_ENET0_E0_1588_PORT_B)
# define CYGHWR_IO_FREESCALE_ENET0_PIN_E0_1588_TMR0  CYGHWR_HAL_KINETIS_PIN(B, 2, 4, 0)
# define CYGHWR_IO_FREESCALE_ENET0_PIN_E0_1588_TMR1  CYGHWR_HAL_KINETIS_PIN(B, 3, 4, 0)
# define CYGHWR_IO_FREESCALE_ENET0_PIN_E0_1588_TMR2  CYGHWR_HAL_KINETIS_PIN(B, 4, 4, 0)
# define CYGHWR_IO_FREESCALE_ENET0_PIN_E0_1588_TMR3  CYGHWR_HAL_KINETIS_PIN(B, 5, 4, 0)
#elif defined(CYGHWR_HAL_FREESCALE_ENET0_E0_1588_PORT_C)
# define CYGHWR_IO_FREESCALE_ENET0_PIN_E0_1588_TMR0  CYGHWR_HAL_KINETIS_PIN(C, 16, 4, 0)
# define CYGHWR_IO_FREESCALE_ENET0_PIN_E0_1588_TMR1  CYGHWR_HAL_KINETIS_PIN(C, 17, 4, 0)
# define CYGHWR_IO_FREESCALE_ENET0_PIN_E0_1588_TMR2  CYGHWR_HAL_KINETIS_PIN(C, 18, 4, 0)
# define CYGHWR_IO_FREESCALE_ENET0_PIN_E0_1588_TMR3  CYGHWR_HAL_KINETIS_PIN(C, 19, 4, 0)
#endif
//=============================================================================
// Memory access checks.
//
// Accesses to areas not backed by real devices or memory can cause
// the CPU to hang. These macros allow the GDB stubs to avoid making
// accidental accesses to these areas.

__externC int cyg_hal_stub_permit_data_access( CYG_ADDRESS addr, cyg_uint32 count );

#define CYG_HAL_STUB_PERMIT_DATA_READ(_addr_, _count_) cyg_hal_stub_permit_data_access( _addr_, _count_ )

#define CYG_HAL_STUB_PERMIT_DATA_WRITE(_addr_, _count_ ) cyg_hal_stub_permit_data_access( _addr_, _count_ )

//=============================================================================


//-----------------------------------------------------------------------------
// end of plf_io.h
#endif // CYGONCE_HAL_PLF_IO_H
