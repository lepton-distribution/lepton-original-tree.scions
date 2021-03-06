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

/*===========================================
Compiler Directive
=============================================*/
#ifndef __KERNEL_IO_H__
#define __KERNEL_IO_H__


/*===========================================
Includes
=============================================*/
#include "kernel/core/types.h"
#include "kernel/core/system.h"


/*===========================================
Declaration
=============================================*/

//
#define KERNEL_IO_READWRITE_NOARGS ((uint8_t)(0))
#define KERNEL_IO_READWRITE_ARGS ((uint8_t)(1))

//
#define __kernel_io_read_set_timeout(__desc__,__s__,__ns__)\
ofile_lst[__desc__].read_timeout.tv_sec   = __s__; ofile_lst[__desc__].read_timeout.tv_nsec   = __ns__;\
ofile_lst[__desc__].p_read_timeout = &ofile_lst[__desc__].read_timeout

#define __kernel_io_read_unset_timeout(__desc__) ofile_lst[__desc__].p_read_timeout = (void*)0

#define __kernel_io_read_get_timeout(__desc__,__s__,__ns__) __s__ = ofile_lst[__desc__].read_timeout.tv_sec; __ns__=ofile_lst[__desc__].read_timeout.tv_nsec

//
extern ssize_t kernel_io_read_args(desc_t desc, void *buf, size_t nbyte, uint8_t args_flags, ...);
extern ssize_t kernel_io_write_args(desc_t desc, const void *buf, size_t nbyte, uint8_t args_flags, ...);

extern ssize_t kernel_io_read(desc_t desc, void *buf, size_t nbyte);
extern ssize_t kernel_io_write(desc_t desc, const void *buf, size_t nbyte);
extern off_t kernel_io_lseek(desc_t desc, off_t offset, int whence);

extern int kernel_io_ll_read(desc_t desc,void *buf, size_t nbyte);
extern int kernel_io_ll_write(desc_t desc,const void *buf, size_t nbyte);
extern int kernel_io_ll_lseek(desc_t desc, off_t offset, int origin);
extern int kernel_io_ll_ioctl(desc_t desc,int request,...);
#endif


