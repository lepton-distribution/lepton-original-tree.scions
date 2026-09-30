/*
 * Lepton — noyau statique hôte : table des pilotes (équivalent fixe du dev_mkconf.c généré).
 * Pilotes logiciels du guide §1.1 (dev_part : décision 2026-09-30) et pilotes de l'hôte
 * (disque hdc sur fichier, horloge rtc0).
 */
#include "kernel/core/kernelconf.h"
#include "kernel/fs/vfs/vfsdev.h"

extern dev_map_t dev_null_map;
extern dev_map_t dev_proc_map;
extern dev_map_t dev_cpufs_map;
extern dev_map_t dev_head_map;
extern dev_map_t dev_tty_map;
extern dev_map_t dev_part_map;
extern dev_map_t dev_host_fileflash_map;
extern dev_map_t dev_host_rtc_map;

pdev_map_t const dev_lst[]={
   &dev_null_map,
   &dev_proc_map,
   &dev_cpufs_map,
   &dev_head_map,
   &dev_tty_map,
   &dev_part_map,
   &dev_host_fileflash_map,
   &dev_host_rtc_map
};

pdev_map_t const * pdev_lst=&dev_lst[0];
const char max_dev=sizeof(dev_lst)/sizeof(pdev_map_t);

/* Image cpufs vide (voir dev_dskimg.h). */
const unsigned char filecpu_memory[1]={0};
const unsigned long filecpu_memory_size=0;
