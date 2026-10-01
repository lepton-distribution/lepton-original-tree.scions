/*
The contents of this file are subject to the Mozilla Public License Version 1.1
(the "License"); you may not use this file except in compliance with the License.
You may obtain a copy of the License at http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS" basis,
WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the
specific language governing rights and limitations under the License.

The Original Code is Lepton.

The Initial Developer of the Original Code is Chauvin-Arnoux.
Portions created by Chauvin-Arnoux are Copyright (C) 2011. All Rights Reserved.

Alternatively, the contents of this file may be used under the terms of the eCos GPL license
(the  [eCos GPL] License), in which case the provisions of [eCos GPL] License are applicable
instead of those above. If you wish to allow use of your version of this file only under the
terms of the [eCos GPL] License and not to allow others to use your version of this file under
the MPL, indicate your decision by deleting  the provisions above and replace
them with the notice and other provisions required by the [eCos GPL] License.
If you do not delete the provisions above, a recipient may use your version of this file under
either the MPL or the [eCos GPL] License."
*/


//Contrat du KAL : branche #else de kal.h (documentation Doxygen, macros vides).
//Spécification de ce que chaque kal/backend/<micro-noyau>/kal_backend.h doit définir.
//Non inclus par kal.h : référence pour l'ajout d'un micro-noyau (ajout-coeur.md §5).
//Extrait par kal_split.py (étape extraire-contrat).
#ifndef _KAL_CONTRAT_H
#define _KAL_CONTRAT_H

/**
* structure de contexte utilis par le micro-noyau\n
* cette structure contient gnralement toutes les informations ncessaire  la commutation de tache:\n
*  1) sauvegarde de certains registres du processeur comme le pointeur de pile par exemple.\n
*  2) sauvegarde de du compteur programme.\n
* \n
* ce sont les deux informations les plus importantes pour lepton. Elle permettent de raliser le vfork()
* et la gestion des signaux avec kill().
* \hideinitializer
*/
typedef CONTEXT context_t;

/**
 * definition du prototype de fonction de la tache gre par le micro-noyau
 *
 * \param pthread_name nom de la fonction
 *
 * \hideinitializer
 */
   #define __begin_pthread(pthread_name)

/**
 * definition de la sortie de fonction de la tache gre par le micro-noyau
 *
 * \hideinitializer
 */
   #define __end_pthread()

/**
 * permet de savoir si le task control block tcb est bien celui de la tache courante
 *
 * \param tcb task control block
 *
 * \hideinitializer
 */
   #define __is_thread_self(tcb)

/**
 * permet de sauvegarder le contexte du thread __pthread_ptr dans context.
 *
 * \param context variable de type context_t dans laquelle sera sauvegarde le contexte.
 * \param __pthread_ptr pointeur sur la structure pthread_t du pthread.
 *
 * \note voir les fonctions _sys_vfork(), _sys_krnl_exec(), _sys_exec().
 * \hideinitializer
 */
   #define __bckup_context(context,__pthread_ptr)

/**
 * permet de restaurer le contexte context dans celui du thread __pthread_ptr.
 *
 * \param context variable de type context_t dans laquelle est plac le contexte  restaurer.
 * \param __pthread_ptr pointeur sur la structure pthread_t du pthread.
 *
 * \note voir les fonctions _sys_vfork(), _sys_vfork_exit(), _sys_krnl_exec(), _sys_exec().
 * \hideinitializer
 */
   #define __rstr_context(context,__pthread_ptr)

/**
 * permet de sauvegarder la pile d'un process
 *
 * \param pid pid du process dont il faut sauvegarder la pile.
 *
 * \note voir les fonctions _sys_vfork(), _sys_krnl_exec(), _sys_exec().
 * \hideinitializer
 */
   #define __bckup_stack(pid)

/**
 * permet de restaurer la pile d'un process
 *
 * \param pid pid du process dont il faut restaurer la pile.
 *
 * \note voir les fonctions _sys_vfork(), _sys_krnl_exec(), _sys_exec().
 * \hideinitializer
 */
   #define __rstr_stack(pid)

/**
 * permet de drouter le flux d'excution d'un process par la fonction _sys_kill().
 *
 * \param pid pid du process dont le flux d'excution doit tre drouter.
 * \param sig_handler address de la fonction sighandler() (voir kernel/signal.c)
 *
 * \note le droutage du flux d'excution est obtenue en modifiant l'addresse de retour d'interruption du scheduler.
 * cette addresse de retour est gnralement place dans la structure qui permet de sauvegarder le contexte context_t de la tche
 * lors de l'appel de l'ordonanceur par l'interuption du timer qui contrle la premption (le tick).
 * cette structure de contexte context_t depend du micro-noyau utilis.
 *
 * \hideinitializer
 */
   #define __swap_signal_handler(pid,sig_handler)

/**
 * permet de restaurer, aprs le droutement par __swap_signal_handler(), le flux d'excution d'un process.
 *
 * \param pid pid du process dont le flux d'excution doit tre restaurer.
 */
   #define __exit_signal_handler(pid)

/**
 * permet de rveiller un process.
 *
 * \param pid pid du process que l'on doit rveiller.
 * \note voir _kernel_timer() dans kernel/kernel.c
 */
   #define __set_active_pid(pid)

/**
 * dbut du zone de code atomique (non premptible)
 *
 * \hideinitializer
 */
   #define __atomic_in() OS_EnterRegion(); //stop task switching and the scheduler, kernel timeslice must be set to 0 cooprative mode).

/**
 * dbut du zone de code atomique (non premptible)
 *
 * \hideinitializer
 */
   #define __atomic_out() OS_LeaveRegion(); //restart task switching and scheduler.

/**
 * arrte le timer qui appel rgulirement l'ordonanceur.
 *
 * \note voir _syscall_execve(), _syscall_vfork(), _syscall_kill(), _syscall_exit(), _syscall_atexit(), _syscall_sigexit().
 */
   #define __stop_sched()

/**
 *  redmarre le timer qui appel rgulirement l'ordonanceur.
 *
 * \note voir _syscall_execve(), _syscall_vfork(), _syscall_kill(), _syscall_exit(), _syscall_atexit(), _syscall_sigexit().
 */
   #define __restart_sched()


#endif //_KAL_CONTRAT_H
