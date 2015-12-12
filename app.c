#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "vsfs.h"

int main(int argc, char *argv[])
{
	char diskname[128]; 
	int  dsize; 
	char filename[16][MAXFILENAMESIZE]; 
	int i, n; 
	int fd0, fd1, fd2;       // file handles
	char buf[MAXREADWRITE]; 

	strcpy (filename[0], "file0"); 
	strcpy (filename[1], "file1"); 
	strcpy (filename[2], "file2"); 
	
	if (argc != 3) {
		printf ("usage: app <diskname> <power2size>\n"); 
		exit (1);
	}
       
	strcpy (diskname, argv[1]); 
	dsize = 1 << (atoi (argv[2])); 
	
	if (vsfs_mount (diskname, dsize) != 0) {
		printf ("could not mound %s\n", diskname); 
		exit (1); 
	}
	else 
		printf ("filesystem %s mounted\n", diskname); 

	
	for (i=0; i<3; ++i) {
		if (vsfs_create (filename[i]) != 0) {
			printf ("could not create file %s\n", filename[i]); 
			exit (1); 
		}
		else 
			printf ("file %s created\n", filename[i]); 
	}


	fd0 = vsfs_open (filename[0]); 
	
	if (fd0 == -1) {
		printf ("file open failed: %s\n", filename[0]); 
		exit (1); 
	}
	

	for (i=0; i<100; ++i) {
		n = vsfs_write (fd0, buf, 500);  
		if (n != 500) {
			printf ("vsfs_write failed\n"); 
			exit (1); 
		}
	}

	for (i=0; i<(100*500); ++i) 
	{
		n = vsfs_read (fd0, buf, 1); 
		if (n != 1) {
			printf ("vsfs_read failed\n"); 
			exit(1); 
		}
	}

	
	vsfs_close (fd0); 

	vsfs_umount(); 
	
	return (0);		
}
