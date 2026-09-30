/*
 * Lepton — libc : strerror sur la numérotation errno de Lepton (kernel/core/errno.h).
 * Licence : voir LICENSE (MPL 1.1).
 *
 * Étape 3b (décision 2026-09-30) : l'API applicative est la libc Lepton ; le strerror de newlib
 * ne convient pas (numérotation errno différente : EACCES = 13 chez newlib, 2 chez Lepton).
 * Table indexée par les noms de kernel/core/errno.h, indépendante de leurs valeurs.
 */
#include "kernel/core/errno.h"
#include "lib/libc/string/string.h"

static const char* const __l_errno_msg[] = {
   [0]               = "Success",
   [E2BIG]           = "Argument list too long",
   [EACCES]          = "Permission denied",
   [EADDRINUSE]      = "Address in use",
   [EADDRNOTAVAIL]   = "Address not available",
   [EAFNOSUPPORT]    = "Address family not supported",
   [EAGAIN]          = "Resource temporarily unavailable",
   [EALREADY]        = "Connection already in progress",
   [EBADF]           = "Bad file descriptor",
   [EBADMSG]         = "Bad message",
   [EBUSY]           = "Device or resource busy",
   [ECANCELED]       = "Operation canceled",
   [ECHILD]          = "No child processes",
   [ECONNABORTED]    = "Connection aborted",
   [ECONNREFUSED]    = "Connection refused",
   [ECONNRESET]      = "Connection reset",
   [EDEADLK]         = "Resource deadlock would occur",
   [EDESTADDRREQ]    = "Destination address required",
   [EDOM]            = "Mathematics argument out of domain of function",
   [EDQUOT]          = "Disk quota exceeded",
   [EEXIST]          = "File exists",
   [EFAULT]          = "Bad address",
   [EFBIG]           = "File too large",
   [EHOSTUNREACH]    = "Host is unreachable",
   [EIDRM]           = "Identifier removed",
   [EILSEQ]          = "Illegal byte sequence",
   [EINPROGRESS]     = "Operation in progress",
   [EINTR]           = "Interrupted function call",
   [EINVAL]          = "Invalid argument",
   [EIO]             = "Input/output error",
   [EISCONN]         = "Socket is connected",
   [EISDIR]          = "Is a directory",
   [ELOOP]           = "Too many levels of symbolic links",
   [EMFILE]          = "Too many open files",
   [EMLINK]          = "Too many links",
   [EMSGSIZE]        = "Message too long",
   [EMULTIHOP]       = "Multihop attempted",
   [ENAMETOOLONG]    = "Filename too long",
   [ENETDOWN]        = "Network is down",
   [ENETUNREACH]     = "Network unreachable",
   [ENFILE]          = "Too many open files in system",
   [ENOBUFS]         = "No buffer space available",
   [ENODATA]         = "No message is available on the STREAM head read queue",
   [ENODEV]          = "No such device",
   [ENOENT]          = "No such file or directory",
   [ENOEXEC]         = "Executable file format error",
   [ENOLCK]          = "No locks available",
   [ENOLINK]         = "Link has been severed",
   [ENOMEM]          = "Not enough space",
   [ENOMSG]          = "No message of the desired type",
   [ENOPROTOOPT]     = "Protocol not available",
   [ENOSPC]          = "No space left on device",
   [ENOSR]           = "No STREAM resources",
   [ENOSTR]          = "Not a STREAM",
   [ENOSYS]          = "Function not implemented",
   [ENOTCONN]        = "The socket is not connected",
   [ENOTDIR]         = "Not a directory",
   [ENOTEMPTY]       = "Directory not empty",
   [ENOTSOCK]        = "Not a socket",
   [ENOTSUP]         = "Operation not supported",
   [ENOTTY]          = "Inappropriate I/O control operation",
   [ENXIO]           = "No such device or address",
   [EOPNOTSUPP]      = "Operation not supported on socket",
   [EOVERFLOW]       = "Value too large to be stored in data type",
   [EPERM]           = "Operation not permitted",
   [EPIPE]           = "Broken pipe",
   [EPROTO]          = "Protocol error",
   [EPROTONOSUPPORT] = "Protocol not supported",
   [EPROTOTYPE]      = "Protocol wrong type for socket",
   [ERANGE]          = "Result too large",
   [EROFS]           = "Read-only file system",
   [ESPIPE]          = "Invalid seek",
   [ESRCH]           = "No such process",
   [ESTALE]          = "Stale file handle",
   [ETIME]           = "Stream ioctl() timeout",
   [ETIMEDOUT]       = "Connection timed out",
   [ETXTBSY]         = "Text file busy",
   [EWOULDBLOCK]     = "Operation would block",
   [EXDEV]           = "Improper link",
};

char* __l_strerror(int errnum){
   if(errnum >= 0 && errnum < (int)(sizeof(__l_errno_msg) / sizeof(__l_errno_msg[0]))
      && __l_errno_msg[errnum])
      return (char*)__l_errno_msg[errnum];
   return (char*)"Unknown error";
}
