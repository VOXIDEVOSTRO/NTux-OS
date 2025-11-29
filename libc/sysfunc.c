#include <libc/sysfunc.h>
#include <fs/ramfs.h>

void sys_rmkdir(const char* name){
    ramfs_mkdir(name);
}

void sys_rls(const char* name){
    ramfs_list_dir(name);
}