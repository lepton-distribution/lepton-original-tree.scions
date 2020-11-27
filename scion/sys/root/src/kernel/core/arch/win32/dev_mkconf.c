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
extern dev_map_t  dev_win32_filerom_map;
extern dev_map_t  dev_win32_fileflash_map;
extern dev_map_t  dev_win32_com0_map;
extern dev_map_t  dev_win32_rtc_map;
extern dev_map_t  dev_win32_kb_map;
extern dev_map_t  dev_tty_map;
extern dev_map_t  dev_win32_lcd_matrix_map;
extern dev_map_t  dev_win32_rotary_switch_1_map;
extern dev_map_t  dev_win32_rotary_switch_2_map;
extern dev_map_t  dev_win32_rotary_switch_3_map;
extern dev_map_t  dev_win32_rotary_switch_4_map;


pdev_map_t const dev_lst[]={
&dev_null_map,
&dev_proc_map,
&dev_cpufs_map,
&dev_head_map,
&dev_pipe_map,
&dev_win32_filerom_map,
&dev_win32_fileflash_map,
&dev_win32_com0_map,
&dev_win32_rtc_map,
&dev_win32_kb_map,
&dev_tty_map,
&dev_win32_lcd_matrix_map,
&dev_win32_rotary_switch_1_map,
&dev_win32_rotary_switch_2_map,
&dev_win32_rotary_switch_3_map,
&dev_win32_rotary_switch_4_map
};

pdev_map_t const * pdev_lst=&dev_lst[0];
const char max_dev = sizeof(dev_lst)/sizeof(pdev_map_t);
/*===========================================
Implementation
=============================================*/


/*===========================================
End of Source dev_mkconf.c
=============================================*/
