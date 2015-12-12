#ifndef VSFS_H
#define VSFS_H

#define BLOCKSIZE          4096     // bytes
#define MAXFILECOUNT       128      // files - max that FS can support
#define MAXDISKSIZE        (1<<28)  // 256 MB - max that FS can support
#define MINDISKSIZE        (1<<20)  // 1 MB - min that FS need
#define MAXFILENAMESIZE    128  // characters - max that FS can support
#define MAXBLOCKCOUNT      (MAXDISKSIZE / BLOCKSIZE)
#define MAXOPENFILES       128      // files
#define MAXREADWRITE      1024     // bytes; max read/write amount


// The following will be used by only a formatting program
int vsfs_format(char *vdisk, int dsize);


// The following will be used by a program to work with files
int vsfs_mount (char *vdisk); 
int vsfs_umount(); 
int vsfs_create(char *filename); 
int vsfs_open(char *filename); 
int vsfs_close(int fd); 
int vsfs_delete(char *filename); 
int vsfs_read(int fd, void *buf, int n); 
int vsfs_write(int fd, void *buf, int n); 
int vsfs_truncate(int fd, int size); 
int vsfs_seek(int fd, int offset); 
int vsfs_filesize (int fd); 
void vsfs_print_dir (); 
void vsfs_print_fat (); 

#endif
