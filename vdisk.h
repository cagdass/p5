#ifndef VDISK_H
#define VDISK_H

void vdisk_open (char *name); 
void vdisk_set (int dsize, int dblocksize); 
int  vdisk_getblock (int blocknum, void *buf);
int  vdisk_putplock (int blocknum, void *buf);
void vdisk_close ();

#endif
