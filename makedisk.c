#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <strings.h>
#include <string.h>

#include "vsfs.h"

int main(int argc, char *argv[])
{
    int n, power, size ,ret, i;
    int fd;  
    char vdiskname[128]; 
    char buf[BLOCKSIZE]; 

    printf ("started\n"); 
    

    strcpy (vdiskname, argv[1]); 
    power = atoi(argv[2]); 
    size = 1 << power; 

    printf ("disk %s size %d\n", vdiskname, size); 
    

    ret = creat (vdiskname, O_RDWR); 
    
    if (ret == -1) {
	printf ("could not create disk\n"); 
	exit(1); 
    }

    bzero ((void *)buf, BLOCKSIZE); 

    fd = open (vdiskname, O_RDWR); 
    
    for (i=0; i< (size/BLOCKSIZE); ++i){
	n = write (fd, buf, BLOCKSIZE); 
	if (n != BLOCKSIZE) {
	    printf ("write error\n"); 
	    exit (1); 
	}
    }

    return (0);
}
