set pagination off
set confirm off
set print pretty off
target remote :1234

echo \n### 1. appelant : entree dans le verrou des appels systeme (__mk_syscall)\n
break kernel_syscall_lock
set $bk_lock = $bpnum
continue
delete $bk_lock
printf "tache courante embOS : %s\n", OS_Global.pCurrentTask->sName
bt 4
finish
printf "retour dans l'appelant, tache : %s\n", OS_Global.pCurrentTask->sName
info line *$pc
set $p = __pthread_ptr__
printf "pthread appelant (variable locale de __mk_syscall) : %p\n", $p
tbreak +1
set $bk_caller = $bpnum

echo \n### 2. tache noyau : reveil par evenement embOS, _kernel_syscall\n
break _kernel_syscall
set $bk_ks = $bpnum
continue
printf "tache courante embOS : %s\n", OS_Global.pCurrentTask->sName
bt 3
printf "pthread appelant : %p, pid=%d, irq_nb=0x%x, syscall=%d, reg.data=%p\n", $p, $p->pid, $p->irq_nb, $p->reg.syscall, $p->reg.data
info symbol kernel_syscall_lst[$p->reg.syscall].p_syscall
set $h = kernel_syscall_lst[$p->reg.syscall].p_syscall
delete $bk_ks

echo \n### 3. gestionnaire de l'appel systeme (tache noyau)\n
break *$h
set $bk_h = $bpnum
continue
printf "tache courante embOS : %s\n", OS_Global.pCurrentTask->sName
bt 3
delete $bk_h
finish

echo \n### 4. fin : liberation du verrou par la tache noyau\n
break kernel_syscall_unlock
set $bk_u = $bpnum
continue
printf "tache courante embOS : %s\n", OS_Global.pCurrentTask->sName
bt 3
printf "etat de la trace noyau : %d (KERNEL_SYSCALL_STATUS_END = 2, kernel.h)\n", _g_kernel_syscall_trace.kernel_syscall_status
delete $bk_u

echo \n### 5. appelant : reprise apres __wait_ret_int\n
continue
printf "tache courante embOS : %s\n", OS_Global.pCurrentTask->sName
info line *$pc
printf "errno de l'appelant : %d\n", $p->_errno
bt 3
kill
quit
