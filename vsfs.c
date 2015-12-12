#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "vsfs.h"

// Global Variables
char disk_name[128]; 
int  disk_size; 
int  disk_fd; 
int  disk_blockcount; 



int getblock (int blocknum, void *buf)
{      
	int offset, n; 
	
	if (blocknum >= disk_blockcount) 
		return (-1); //error

	offset = lseek (disk_fd, blocknum * BLOCKSIZE, SEEK_SET); 
	n = read (disk_fd, buf, BLOCKSIZE); 
	if (n != BLOCKSIZE) 
		return (-1); 

	return (0); 
}

int putblock (int blocknum, void *buf)
{
	int offset, n;
	
	if (blocknum >= disk_blockcount) 
		return (-1); //error

	offset = lseek (disk_fd, blocknum * BLOCKSIZE, SEEK_SET); 
	n = write (disk_fd, buf, BLOCKSIZE); 
	if (n != BLOCKSIZE) 
		return (-1); 

	return (0); 
}





int vsfs_format(char *vdisk, int dsize)
{
	strcpy (disk_name, vdisk); 
	disk_size = dsize;  
	disk_blockcount = disk_size / BLOCKSIZE; 


	disk_fd = open (disk_name, O_RDWR); 
	if (disk_fd == -1) {
		printf ("disk open error %s\n", vdisk); 
		exit(1); 
	}
	
	// perform your format operations here. 
	
	printf ("formatting disk=%s, size=%d\n", vdisk, disk_size); 

	fsync (disk_fd); 
	close (disk_fd); 

	return (0); 
}


int vsfs_mount (char *vdisk)
{
	unsigned char buffer[BLOCKSIZE]; 
	struct stat finfo; 

	strcpy (disk_name, vdisk);
	disk_fd = open (disk_name, O_RDWR); 
	if (disk_fd == -1) {
		printf ("vsfs_mount: disk open error %s\n", disk_name); 
		exit(1); 
	}
	
	fstat (disk_fd, &finfo); 

	printf ("vsfs_mount: mounting %s, size=%d\n", disk_name, 
		(int) finfo.st_size);  
	disk_size = (int) finfo.st_size; 
	disk_blockcount = disk_size / BLOCKSIZE; 

	// perform your mount operations here

	return (0); 
}


int vsfs_umount()
{
	// perform your mount operations here

	fsync (disk_fd); 
	close (disk_fd); 
}

int vsfs_create(char *filename)
{

	return (0); 
}

int vsfs_open(char *filename)
{
	

}

int vsfs_close(int fd)
{

}

int vsfs_delete(char *filename)
{


}

int vsfs_read(int fd, void *buf, int n)
{

	
	return (n); // if everything is OK

}

int vsfs_write(int fd, void *buf, int n)
{


	return (n); // if everything is OK
} 

int vsfs_truncate(int fd, int size)
{

} 


int vsfs_seek(int fd, int offset)
{

} 


void vsfs_print_dir ()
{

}


void vsfs_print_fat ()
{

}


