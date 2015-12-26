#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "vsfs.h"

/* 
#define BLOCKSIZE          4096     // bytes
#define MAXFILECOUNT       128      // files - max that FS can support
#define MAXDISKSIZE        (1<<28)  // 256 MB - max that FS can support
#define MINDISKSIZE        (1<<20)  // 1 MB - min that FS need
#define MAXFILENAMESIZE    128  // characters - max that FS can support
#define MAXBLOCKCOUNT      (MAXDISKSIZE / BLOCKSIZE)
#define MAXOPENFILES       128      // files
#define MAXREADWRITE      1024     // bytes; max read/write amount
*/

//create it as such all free pointers point to their next
//structure
//entry -> number of files
//for each file:
//entry -> filename, pointer to the beginning
//for each block, pointer to the next

// Global Variables
char disk_name[128];   // name of virtual disk file
int  disk_size;        // size in bytes - a power of 2
int  disk_fd;          // disk file handle
int  disk_blockcount;  // block count on disk
int  blocks_allocated = 0;
int  files_allocated = 0;
int  first_free_block = 0;
// Pointers & Data for FAT Table
char** filenames;
int* initial_pointers;
int* block_pointers;
int* offset_pointers;
int* open_files;

/* 
   Reads block blocknum into buffer buf.
   You will not modify the getblock() function. 
   Returns -1 if error. Should not happen.
*/
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


/*  
    Puts buffer buf into block blocknum.  
    You will not modify the putblock() function
    Returns -1 if error. Should not happen. 
*/
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





/* 
   IMPLEMENT THE FUNCTIONS BELOW - You can implement additional 
   internal functions. 
 */



/* format disk of size dsize */
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
	FILE* fp = fopen(disk_name, "rw+");
	
	// write the number of blocks and files in the system
	fprintf(fp, "%d\n", 0);
	fprintf(fp, "%d\n", 0);
	// write first free block's address
	fprintf(fp, "%d\n", first_free_block);
	
	// write names of the files in each line and the pointers to their first locations in the next line
	// also write the file position pointers
	for(i = 0; i < MAXFILECOUNT; i++){
		fprintf(fp, "free\n");
		fprintf(fp, "%d\n", -1);
		fprintf(fp, "%d\n", 0);
	}

	// initialize free block pointers
	for(i = 0; i < disk_size / BLOCKSIZE - 1; i++){
		fprintf(fp, "%d\n", i+1);
	}
	// the last free block pointer initially points to the beginning
	fprintf(fp, "%d", 0);

	fclose(fp);



	fsync (disk_fd); 
	close (disk_fd); 

	return (0); 
}

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
	block_pointers = (int*)malloc(disk_size / BLOCKSIZE * sizeof(int));
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
	for(i = 0; i < disk_size / BLOCKSIZE; i++){
		block_pointers[i] = -1;
	}

	FILE* fp = fopen(disk_name, "r");

	char * line = NULL;
	size_t len = 0;

	getline(&line, &len, fp);
	blocks_allocated = atoi(line);
	getline(&line, &len, fp);
	files_allocated = atoi(line);
	getline(&line, &len, fp);
	first_free_block = atoi(line);
	for(i = 0; i < MAXFILECOUNT; i++){
		getline(&line, &len, fp);
		printf("Filenames read %d: %s\n", i, line);
		filenames[i] = line;
		getline(&line, &len, fp);
		initial_pointers[i] = atoi(line);
		getline(&line, &len, fp);
		offset_pointers[i] = atoi(line);
	}
	for(i = 0; i < disk_size / BLOCKSIZE; i++){
		getline(&line, &len, fp);
		block_pointers[i] = atoi(line);
	}

	fclose(fp);

  	return (0); 
}


int vsfs_umount()
{
	FILE* fp = open(disk_name, "rw+");
	fprintf(fp, "%d\n", blocks_allocated);
	fprintf(fp, "%d\n", files_allocated);
	fprintf(fp, "%d\n", first_free_block);

	int i;

	for(i = 0; i < MAXFILECOUNT; i++){
		fprintf(fp, "%s\n", filenames[i]);
		fprintf(fp, "%d\n", initial_pointers[i]);
		fprintf(fp, "%d\n", offset_pointers[i]);
	}
	for(i = 0; i < disk_size / BLOCKSIZE; i++){
		fprintf(fp, "%d\n", block_pointers[i]);
	}

	// free the allocated space in memory
	for(i = 0; i < MAXFILECOUNT; i++){
		free(filenames[i]);
	}
	free(filenames);
	free(initial_pointers);
	free(offset_pointers);
	free(block_pointers);

	fsync (disk_fd); 
	close (disk_fd); 

	return (0); 
}


/* create a file with name filename */
int vsfs_create(char *filename)
{
	if(files_allocated < MAXFILECOUNT){
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
				initial_pointers[i] = first_free_block;
				first_free_block = block_pointers[first_free_block];
				block_pointers[initial_pointers[i]] = -1;
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
	int bytes_read = -1; 


	
	return (bytes_read); 

}

int vsfs_write(int fd, void *buf, int n)
{
	int bytes_written = -1; 

	// write your code

	return (bytes_written); 
} 

int vsfs_truncate(int fd, int size)
{

	// write your code

	return (0); 
} 


int vsfs_seek(int fd, int offset)
{
	int position = -1; 

	// write your code

	return (position); 
} 

int vsfs_filesize (int fd)
{
	int size = -1;
	int ptr = initial_pointers[ptr];
	size = 0;
	while(1){
		if(ptr = -1){
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


