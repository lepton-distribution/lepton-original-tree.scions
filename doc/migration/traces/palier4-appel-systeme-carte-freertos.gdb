set pagination off
set confirm off
set print pretty off
# variante carte FreeRTOS (étape 7, module 7.2, NUCLEO-F439ZI ; copie de
# palier4-appel-systeme-carte.gdb, tâche courante par pxCurrentTCB) : OpenOCD (debug/openocd-nucleo-f439zi.cfg), port 3333 ;
# palier 3 (warmup) puis palier 4 ; points d'arrêt matériels (6 au plus)
target extended-remote :3333
monitor reset halt

echo \n### 0. palier 3 : warmup du noyau\n
break _kernel_warmup_boot
set $bk_w = $bpnum
continue
bt 3
delete $bk_w

echo \n### 1. appelant : entree dans le verrou des appels systeme (__mk_syscall)\n
break kernel_syscall_lock
set $bk_lock = $bpnum
continue
delete $bk_lock
printf "tache courante FreeRTOS : %s\n", pxCurrentTCB->pcTaskName
bt 4
finish
printf "retour dans l'appelant, tache : %s\n", pxCurrentTCB->pcTaskName
info line *$pc
set $p = __pthread_ptr__
printf "pthread appelant (variable locale de __mk_syscall) : %p\n", $p
tbreak +1
set $bk_caller = $bpnum

echo \n### 2. tache noyau : reveil par groupe d'evenements FreeRTOS, _kernel_syscall\n
break _kernel_syscall
set $bk_ks = $bpnum
continue
printf "tache courante FreeRTOS : %s\n", pxCurrentTCB->pcTaskName
bt 3
printf "pthread appelant : %p, pid=%d, irq_nb=0x%x, syscall=%d, reg.data=%p\n", $p, $p->pid, $p->irq_nb, $p->reg.syscall, $p->reg.data
info symbol kernel_syscall_lst[$p->reg.syscall].p_syscall
set $h = kernel_syscall_lst[$p->reg.syscall].p_syscall
delete $bk_ks

echo \n### 3. gestionnaire de l'appel systeme (tache noyau)\n
break *$h
set $bk_h = $bpnum
continue
printf "tache courante FreeRTOS : %s\n", pxCurrentTCB->pcTaskName
bt 3
delete $bk_h
finish

echo \n### 4. fin : liberation du verrou par la tache noyau\n
break kernel_syscall_unlock
set $bk_u = $bpnum
continue
printf "tache courante FreeRTOS : %s\n", pxCurrentTCB->pcTaskName
bt 3
printf "etat de la trace noyau : %d (KERNEL_SYSCALL_STATUS_END = 2, kernel.h)\n", _g_kernel_syscall_trace.kernel_syscall_status
delete $bk_u

echo \n### 5. appelant : reprise apres __wait_ret_int\n
continue
printf "tache courante FreeRTOS : %s\n", pxCurrentTCB->pcTaskName
info line *$pc
printf "errno de l'appelant : %d\n", $p->_errno
bt 3
monitor reset run
detach
quit
