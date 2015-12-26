#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "vsfs.h"

//create it as such all free pointers point to their next
//structure
//entry -> number of files
//for each file:
//entry -> filename, pointer to the beginning
//for each block, pointer to the next

// the last two blocks are always reserved to the fat system

// Global Variables
char disk_name[128];   // name of virtual disk file
int  disk_size;        // size in bytes - a power of 2
int  disk_fd;          // disk file handle
int  disk_blockcount;  // block count on disk

// additional global variables
int  blocks_allocated = 0;
int  files_allocated = 0;
int  first_free_block = 0;

// struct to hold data about the file attributes
struct node {
	char filename[128];
	// pointer to the first block of the file
	int initial_pointer;
	// pointer to the last modified location inside a file
	int offset_pointer;
	// to indicate whether the file is open or not
	int open;
};

// Pointers & Data for FAT Table when it is brought to the memory
char** filenames;
int* initial_pointers;
int* block_pointers;
int* offset_pointers;
int* open_files;

int getblock (int blocknum, void *buf)
{      
	int offset, n; 
	
	if (blocknum >= disk_blockcount) 
		return (-1); //error
	offset = lseek (disk_fd, blocknum * BLOCKSIZE, SEEK_SET); 
	if (offset == -1) {
		printf ("lseek error\n"); 
		exit(0); 

	}
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
	if (offset == -1) {
		printf ("lseek error\n"); 
		exit (1); 
	}
	n = write (disk_fd, buf, BLOCKSIZE); 
	if (n != BLOCKSIZE) 
		return (-1); 

	return (0); 
}

// formate the vsfs disk
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
	
	int i;

	struct node* buffer1 = (struct node*)malloc(BLOCKSIZE);
	int* buffer2 = (int*)malloc(BLOCKSIZE);
	
	// write the number of blocks and files in the system
	
	// write names of the files in each line and the pointers to their first locations in the next line
	// also write the file position pointers
	struct node ptr;
	for(i = 0; i < MAXFILECOUNT; i++){
		// initialize fcb's attributes
		ptr = buffer1[i];
		strcpy(ptr.filename, "");
		ptr.initial_pointer = -1;
		ptr.offset_pointer = 0;
		ptr.open = 0;
	}

	// put the block in the memory
	putblock(disk_size / BLOCKSIZE - 2, (void*)buffer1);
	free(buffer1);


	// initialize free block pointers
	for(i = 0; i < disk_size / BLOCKSIZE - 3; i++){
		buffer2[i] = i+1;
	}

	putblock(disk_size / BLOCKSIZE - 1, buffer2);
	free(buffer2);

	fsync (disk_fd); 
	close (disk_fd); 

	return (0); 
}

// bring the fat data from disk to the memory
int vsfs_mount (char *vdisk)
{
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

	int i; // loop variable

	// create an array to keep the filenames, (has 128 elements just in case maximum file count times files are created)
	filenames = (char**)malloc(MAXFILECOUNT * sizeof(char*));
	// create a list of offsets for to add the mount address to find which file's beginning points to where
	initial_pointers = (int*)malloc(MAXFILECOUNT * sizeof(int));
	// create a list of offsets for all the blocks
	block_pointers = (int*)malloc( ((disk_size / BLOCKSIZE) -2 ) * sizeof(int));
	// file position pointers
	offset_pointers = (int*)malloc(MAXFILECOUNT * sizeof(int));
	// a list of open files, indexed corresponding to the filenames list
	open_files = (int*)malloc(MAXFILECOUNT * sizeof(int));

	for(i = 0; i < MAXFILECOUNT; i++){
		// allocate space for the names of the files
		filenames[i] = (char*)malloc(MAXFILENAMESIZE * sizeof(char));
		// if 0 zero file is not opened, if 1 file is open
		open_files[i] = 0;
	}

	// let all the pointers point to -1 first, depicting they point to null
	for(i = 0; i < MAXFILECOUNT; i++){
		initial_pointers[i] = -1;
	}

	// let all the pointers point to -1 first, depicting they point to null
	for(i = 0; i < disk_size / BLOCKSIZE - 2; i++){
		block_pointers[i] = -1;
	}

	// these pointers are reserved by fat table
	// this block holds the fcb of the created files
	block_pointers[disk_size / BLOCKSIZE - 2] = -1;
	// this block holds the pointer information of the blocks
	block_pointers[disk_size / BLOCKSIZE - 1] = -1;

	struct node* buffer1;
	
	buffer1 = (struct node*)malloc(BLOCKSIZE);

	// get the file control block into the memory 
	getblock(disk_size / BLOCKSIZE - 2, (void*)buffer1);

	struct node ptr;

	// copy the fcb information into the memory
	for(i = 0; i < MAXFILECOUNT; i++){
		ptr = buffer1[i];
		strcpy(filenames[i], ptr.filename);
		initial_pointers[i] = ptr.initial_pointer;
		offset_pointers[i] = ptr.offset_pointer;
		open_files[i] = ptr.open;
	}

	free(buffer1);

	int* buffer2 = (int*)malloc(BLOCKSIZE);
	getblock(disk_size / BLOCKSIZE - 1, (void*)buffer2);

	// copy block pointer information to the memory
	for(i = 0; i < (disk_size / BLOCKSIZE) - 2; i++){
		block_pointers[i] = buffer2[i];
	}

	free(buffer2);

  	return (0); 
}


// write the changes in the memory related to fat into the disk
int vsfs_umount()
{
	int i;

	struct node* buffer1 = (struct node*)malloc(BLOCKSIZE);
	int* buffer2 = (int*)malloc(BLOCKSIZE);
	
	// write the number of blocks and files in the system

	// write names of the files in each line and the pointers to their first locations in the next line
	// also write the file position pointers
	struct node ptr;
	for(i = 0; i < MAXFILECOUNT; i++){
		// filename
		ptr = buffer1[i];
		strcpy(ptr.filename, filenames[i]);
		ptr.initial_pointer = initial_pointers[i];
		ptr.open = open_files[i];
		ptr.offset_pointer = offset_pointers[i];
	}

	putblock(disk_size / BLOCKSIZE - 2, buffer1);
	free(buffer1);

	// initialize free block pointers
	for(i = 0; i < disk_size / BLOCKSIZE - 2; i++){
		buffer2[i] = (block_pointers[i]);
	}
	putblock(disk_size / BLOCKSIZE - 1, buffer2);
	free(buffer2);

	fsync (disk_fd); 
	close (disk_fd); 

	return (0); 
}


/* create a file with name filename */
int vsfs_create(char *filename)
{
	if(files_allocated < MAXFILECOUNT && blocks_allocated < disk_size / BLOCKSIZE - 2){
		// create the file and return an error message otherwise
		int ret = open (filename,  O_CREAT | O_RDWR, 0666);
		if (ret == -1) {
			printf ("could not create file\n"); 
			exit(1); 
		}

		int i;

		// create a record for the newly created file in our data structures
		for(i = 0; i < MAXFILECOUNT; i++){
			if(strcmp(filenames[i], "free") == 0){
				printf("Hola %d\n", i);
				strcpy(filenames[i], filename);
				filenames[i] = filename;
				// first free block is allocated for the newly created file
				initial_pointers[i] = first_free_block;
				// the new first block will be the block that is pointed by the previous first block
				first_free_block = block_pointers[first_free_block];
				// our file's first block points to nothing
				block_pointers[initial_pointers[i]] = -1;
				// increment the following
				files_allocated++;
				blocks_allocated++;
				break;
			}
		}

		return (0); 
	}
	else{
		printf("Cannot create file. Number of allowed files is reached\n");
		exit(1);
	}

}


/* open file filename */
int vsfs_open(char *filename)
{
	int index = -1; 
	int i;
	// go through all the files and compare the input filename with the filenames in our list
	for(i = 0; i < MAXFILECOUNT; i++){
		printf("%d %s %s\n", i, filenames[i], filename);
		if(strcmp(filenames[i], filename) == 0){
			// file is found, indicate the file is open and return the index
			open_files[i] = 1;
			return i;
		}
	}
       
	return (index); 
}

/* close file filename */
int vsfs_close(int fd)
{
	// indicate the file is close in the open files list
	open_files[fd] = 0;

	return (0); 
}

int vsfs_delete(char *filename)
{
	int index = -1;
	int i;
	for(i = 0; i < MAXFILECOUNT; i++){
		if(strcmp(filenames[i], filename) == 0){
			open_files[i] = 0;
			// save the index
			index = i;
			// decrement the number of files allocated
			files_allocated--;
			break;
		}
	}

	// beginning from the first block of the file
	int ptr = initial_pointers[index];
	while(1){
		blocks_allocated--;
		int temp = block_pointers[ptr];
		block_pointers[ptr] = first_free_block;
		first_free_block = ptr;
		ptr = temp;
		if(ptr < 0 || ptr >= disk_size / BLOCKSIZE){
			break;
		}
	}

	return (0); 
}

int vsfs_read(int fd, void *buf, int n)
{
	int bytes_read = 0;
	int ptr = initial_pointers[fd];

	while(bytes_read < n && ptr != -1){
		getblock(ptr, buf);
		ptr = block_pointers[ptr];
		bytes_read += BLOCKSIZE;
	}
	
	return (bytes_read); 

}

int vsfs_write(int fd, void *buf, int n)
{
	int bytes_written = 0; 
	// int num_blocks = n / BLOCKSIZE;
	// int offset = n % BLOCKSIZE;
	int ptr = initial_pointers[fd];

	while(bytes_written < n && ptr != -1){
		putblock(ptr, buf);
		ptr = block_pointers[ptr];
		bytes_written += BLOCKSIZE;
	}

	return (bytes_written); 
} 

int vsfs_truncate(int fd, int size)
{
	int ptr = initial_pointers[fd];
	int current_size = 0;
	int temp = -1;
	while(current_size < size && ptr != -1){
		temp = first_free_block;
		first_free_block = ptr;
		initial_pointers[fd] = block_pointers[block_pointers[ptr]];
		block_pointers[ptr] = temp;
		ptr = initial_pointers[fd];
		current_size += BLOCKSIZE;
		blocks_allocated--;
	}

	return (0); 
} 


int vsfs_seek(int fd, int offset)
{
	// find current offset
	int ptr = initial_pointers[fd];
	int num_blocks = offset / BLOCKSIZE;
	// int block_offset = offset % BLOCKSIZE;

	int block_count = 1;

	while(1){
		if(ptr == -1){
			offset_pointers[fd] = block_count * BLOCKSIZE;
			break;
		}
		else if(block_count == num_blocks){
			offset_pointers[fd] = offset;
		}
		ptr = block_pointers[ptr];
		block_count++;
	}

	return (offset_pointers[fd]); 
} 

int vsfs_filesize (int fd)
{
	int size = -1;
	int ptr = initial_pointers[fd];
	size = 0;
	while(1){
		if(ptr == -1){
			break;
		}
		ptr = block_pointers[ptr];
		size += 1;
	}

	return size * BLOCKSIZE;
}


void vsfs_print_dir ()
{
	int i;
	// print the name of the files that are not null
	for(i = 0; i < MAXFILECOUNT; i++){
		if(filenames[i] != NULL){
			printf("%s\n", filenames[i]);	
		}
	}
}


void vsfs_print_fat ()
{
	int i;
	// go through the files here
	for(i = 0; i < MAXFILECOUNT; i++){
		if(filenames[i] != NULL){
			// print file name
			printf("%s: ", filenames[i]);
			// get the pointer that points to the first block of the file and iterate the pointers until the last block is reached
			int ptr = initial_pointers[i];
			while(1)
			{
				// print block value
				printf("%d ", ptr);
				ptr = block_pointers[ptr];
				if(ptr < 0 || ptr >= disk_size / BLOCKSIZE){
					break;
				}
			}
			// print new line since the new file will be begun printing after this
			printf("\n");
		}
	}

}


