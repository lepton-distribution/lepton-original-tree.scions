/*-------------------------------------------
| Copyright(C) 2007 CHAUVIN-ARNOUX
---------------------------------------------
| Project:
| Project Manager:
| Source: mklepton.c
| Path: X:\TOOLS\mklepton
| Authors:
| Plateform:
| Created:
| Revision/Date:
| Description:
---------------------------------------------
| Historic:
---------------------------------------------
| Authors	| Date	| Comments
---------------------------------------------*/


/*===========================================
Includes
=============================================*/
#include "kernel/core/bin.h"
//see "sbin/initd.c"
int initd_main(int argc, char* argv[]);

//see "sbin/lsh.c"
int lsh_main(int argc, char* argv[]);

//see "sbin/mount.c"
int mount_main(int argc, char* argv[]);

//see "sbin/umount.c"
int umount_main(int argc, char* argv[]);

//see "sbin/ls.c"
int ls_main(int argc, char* argv[]);

//see "sbin/ps.c"
int ps_main(int argc, char* argv[]);

//see "sbin/kill.c"
int kill_main(int argc, char* argv[]);

//see "sbin/touch.c"
int touch_main(int argc, char* argv[]);

//see "sbin/cat.c"
int cat_main(int argc, char* argv[]);

//see "sbin/more.c"
int more_main(int argc, char* argv[]);

//see "sbin/mkfs.c"
int mkfs_main(int argc, char* argv[]);

//see "sbin/df.c"
int df_main(int argc, char* argv[]);

//see "sbin/uname.c"
int uname_main(int argc, char* argv[]);

//see "sbin/pwd.c"
int pwd_main(int argc, char* argv[]);

//see "sbin/rmdir.c"
int rmdir_main(int argc, char* argv[]);

//see "sbin/mkdir.c"
int mkdir_main(int argc, char* argv[]);

//see "sbin/rm.c"
int rm_main(int argc, char* argv[]);

//see "sbin/od.c"
int od_main(int argc, char* argv[]);

//see "sbin/date.c"
int date_main(int argc, char* argv[]);

//see "sbin/sleep.c"
int sleep_main(int argc, char* argv[]);

//see "sbin/echo.c"
int echo_main(int argc, char* argv[]);

//see "sbin/stty.c"
int stty_main(int argc, char* argv[]);

//see "bin/nu.c"
int nu_main(int argc, char* argv[]);

//see "bin/nuodio/nu.c"
int nu_main(int argc, char* argv[]);



/*===========================================
Global Declaration
=============================================*/
static const bin_t _bin_lst[]={
{         "initd",			                 initd_main,			100,			2048,			1},
{           "lsh",			                   lsh_main,			100,			2048,			1},
{         "mount",			                 mount_main,			100,			2048,			1},
{        "umount",			                umount_main,			100,			2048,			1},
{            "ls",			                    ls_main,			100,			2048,			1},
{            "ps",			                    ps_main,			100,			2048,			1},
{          "kill",			                  kill_main,			100,			2048,			1},
{         "touch",			                 touch_main,			100,			2048,			1},
{           "cat",			                   cat_main,			100,			2048,			1},
{          "more",			                  more_main,			100,			2048,			1},
{          "mkfs",			                  mkfs_main,			100,			2048,			1},
{            "df",			                    df_main,			100,			2048,			1},
{         "uname",			                 uname_main,			100,			2048,			1},
{           "pwd",			                   pwd_main,			100,			2048,			1},
{         "rmdir",			                 rmdir_main,			100,			2048,			1},
{         "mkdir",			                 mkdir_main,			100,			2048,			1},
{            "rm",			                    rm_main,			100,			2048,			1},
{            "od",			                    od_main,			100,			2048,			1},
{          "date",			                  date_main,			100,			2048,			1},
{         "sleep",			                 sleep_main,			100,			2048,			1},
{          "echo",			                  echo_main,			100,			2048,			1},
{          "stty",			                  stty_main,			100,			2048,			1},
{            "nu",			                    nu_main,			130,			2048,			1},
{            "nu",			                    nu_main,			130,			2048,			1}
};

const int bin_lst_size   = sizeof(_bin_lst)/sizeof(bin_t);
const bin_t* bin_lst = &_bin_lst[0];


/*===========================================
Implementation
=============================================*/

/*===========================================
End of Source mklepton.c
=============================================*/
