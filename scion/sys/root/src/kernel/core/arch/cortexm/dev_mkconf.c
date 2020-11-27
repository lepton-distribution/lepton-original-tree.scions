/*-------------------------------------------
| Copyright(C) 2007 CHAUVIN-ARNOUX
---------------------------------------------
| Project:
| Project Manager:
| Source: dev_mkconf.c
| Path: X:\sources\kernel\config
| Authors:
| Plateform:
| Created:
| Revision/Date: $Revision: 1.1 $  $Date: 2009-03-30 15:49:10 $
| Description:
---------------------------------------------
| Historic:
---------------------------------------------
| Authors	| Date	| Comments
 -
---------------------------------------------*/


/*===========================================
Includes
=============================================*/
#include "kernel/core/kernelconf.h"
#include "kernel/fs/vfs/vfsdev.h"


/*===========================================
Global Declaration
=============================================*/


extern dev_map_t  dev_null_map;
extern dev_map_t  dev_proc_map;
extern dev_map_t  dev_cpufs_map;
extern dev_map_t  dev_head_map;
extern dev_map_t  dev_pipe_map;
extern dev_map_t  dev_mem_map;
extern dev_map_t  dev_lwip_core_socket_map;
extern dev_map_t  dev_lwip_stack_map;
extern dev_map_t  dev_samv7x_cpu_x_map;
extern dev_map_t  dev_same70xplained_board_map;
extern dev_map_t  dev_samv71xplained_ultra_usart_1_map;
extern dev_map_t  dev_samv71xplained_ultra_uart_4_map;
extern dev_map_t  dev_same70xplained_i2c_0_map;
extern dev_map_t  dev_samv7x_adc0_map;
extern dev_map_t  dev_samv7x_eth_map;


pdev_map_t const dev_lst[]={
&dev_null_map,
&dev_proc_map,
&dev_cpufs_map,
&dev_head_map,
&dev_pipe_map,
&dev_mem_map,
&dev_lwip_core_socket_map,
&dev_lwip_stack_map,
&dev_samv7x_cpu_x_map,
&dev_same70xplained_board_map,
&dev_samv71xplained_ultra_usart_1_map,
&dev_samv71xplained_ultra_uart_4_map,
&dev_same70xplained_i2c_0_map,
&dev_samv7x_adc0_map,
&dev_samv7x_eth_map
};

pdev_map_t const * pdev_lst=&dev_lst[0];
const char max_dev = sizeof(dev_lst)/sizeof(pdev_map_t);
/*===========================================
Implementation
=============================================*/


/*===========================================
End of Source dev_mkconf.c
=============================================*/
