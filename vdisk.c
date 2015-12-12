
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "vdisk.h"

char disk_name[128]; 
int  disk_fd;
int  disk_size; 
int  disk_blocksize; 
int  disk_blockcount; 

void vdisk_open (char *name)
{
	strcpy (disk_name, name); 

	disk_fd  = open (disk_name, O_RDWR); 
	if (disk_fd == -1) {
		printf ("disk open failed %s\n", disk_name); 
		exit (1); 
	}
}


void vdisk_set (int dsize, int dblocksize)
{
	disk_size = dsize; 
	disk_blocksize = dblocksize; 
	disk_blockcount = disk_size / disk_blocksize; 
}

int vdisk_getblock (int blocknum, void *buf)
{      
	int offset, n; 
	
	if (blocknum >= disk_blockcount) 
		return (-1); //error

	offset = lseek (disk_fd, blocknum * disk_blocksize, SEEK_SET); 
	n = read (disk_fd, buf, disk_blocksize); 
	if (n != disk_blocksize) 
		return (-1); 

	return (0); 
}

int vdisk_putplock (int blocknum, void *buf)
{
	int offset, n;
	
	if (blocknum >= disk_blockcount) 
		return (-1); //error

	offset = lseek (disk_fd, blocknum * disk_blocksize, SEEK_SET); 
	n = write (disk_fd, buf, disk_blocksize); 
	if (n != disk_blocksize) 
		return (-1); 

	return (0); 
}

void vdisk_close ()
{
	fsync (disk_fd); 
	close (disk_fd);       
}

