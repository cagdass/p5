
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "vsfs.h"

int main (int argc, char *argv[])
{
	char vdiskname [128]; 
	int power; 
	int size; 
	
	if (argc != 3) {
		printf ("usage: formatdisk <vdiskname> <power2size>\n"); 
		exit (1); 
	}

	strcpy (vdiskname, argv[1]); 
	power = atoi (argv[2]);
	size = 1 << power;
	vsfs_format (vdiskname, size); 
	return (0); 
}
