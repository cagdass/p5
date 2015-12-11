#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "vsfs.h"


int vsfs_format(char *vdiskname); 
int vsfs_mount (char *vdiskname); 
int vsfs_umount(char *vdiskname); 
int vsfs_create(char *filename); 
int vsfs_open(char *filename); 
int vsfs_close(int fd); 
int vsfs_delete(char *filename); 
int vsfs_read(int fd, void *buf, int n); 
int vsfs_append(int fd, void *buf, int n); 
int vsfs_truncate(int fd, int size); 
int vsfs_seek(int fd, int offset); 

