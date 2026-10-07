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


//KAL, axe ISA : hôte (noyau statique de mklepton), x86_64 ou autre ABI.
//Extrait de kal.h (kal_split.py, étape extraire-static) ; sélectionné par cmake/isa/host.cmake.
#ifndef _KAL_ARCH_HOST_H
#define _KAL_ARCH_HOST_H

   #define __va_list_copy(__dest_va_list__,__src_va_list__) va_copy(__dest_va_list__,__src_va_list__)

   //va_list reçu par argument variadique (vfs.c, I_LINK). x86_64 : va_list est un tableau,
   //l'appelant a donc transmis un pointeur vers son va_list (étape 8).
   #if defined(__x86_64__)
      #define __va_list_from_arg(__dest_va_list__,__ap__) va_copy(__dest_va_list__,*(va_list *)va_arg(__ap__, void *))
   #else
      #define __va_list_from_arg(__dest_va_list__,__ap__) __dest_va_list__ = va_arg(__ap__, va_list)
   #endif

#endif //_KAL_ARCH_HOST_H
