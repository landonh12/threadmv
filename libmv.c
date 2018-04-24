// Landon Haugh
// lrh282
// libmv.c
//
// Library for custom "mv" functions
// Compiled into a *.o file

#include "libmv.h"
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

void* movefile(void *ptr) {
	
	// Create data struct from data passed by pointer from main code
	struct thread_data *data;
	data = (struct thread_data*)ptr;
	
	// Create rusage struct
	struct rusage usage;
    int usage_time = 0;

	// Size of file, assign blocksize from arguments
	double size = 0;
	size = getfilesize(data->filename);
  	int blocksize = data->blocksize;

	// Create buffer for read()/write() commands
	char* buf;
    if(blocksize != -1) {
		buf = malloc(sizeof(char)*blocksize);
	} else {
		buf = malloc(sizeof(char)*size);
	}

	// File descriptors
	int outft, inft;

	// Destination directory allocation
	char* dest_file = data->dest_dir;
	strcat(dest_file, data->filename);
	
	// Open output and input files
	if((outft = open(data->dest_dir, O_CREAT | O_APPEND | O_WRONLY, 0666)) == -1) {
		perror("open");
	}
	inft = open(data->filename, O_RDONLY);
    
	// Error checking for input file
	if(inft > 0) {
		// Loop through file and read/write specified blocksize if blocksize != -1
		if(blocksize != -1) {
			for(int i = 0; i < size; i = i + blocksize) {
				read(inft, buf, blocksize);
				write(outft, buf, blocksize);
			}
		// Normal read/write - copy entire file into buffer
		} else {
			read(inft, buf, size);
			write(outft, buf, size);
		}

		// Get resource usage
		getrusage(RUSAGE_SELF, &usage);
		usage_time = usage.ru_stime.tv_usec;

		// Close input file
		close(inft);
	}
    
	// Close output file
	close(outft);

	// Remove old file
	remove(data->filename);

	// Free dynamically allocated buffer
	free(buf);

	// Send byte size back to main code
	data->bytes = sizeof(char) * size;

	// Print usage_time from rusage
	printf("Time: %d [sec]\n", usage_time);

	// Exit thread
	pthread_exit(NULL);
}


void* mmap_move(void *ptr) {

	// Create data struct from data passed by pointer from main code
	struct thread_data *data;
	data = (struct thread_data*)ptr;

	// Create rusage struct
	struct rusage usage;
	int usage_time = 0;

	// Get filesize
	double size = 0;
	size = getfilesize(data->filename);
	
	// Destination file allocation
	char* dest_file = data->dest_dir;
	strcat(dest_file, data->filename);

	// Create input/output file descriptors and error check
	int inft;
	if((inft = open(data->filename, O_RDWR, (mode_t)0600)) == -1) {
		perror("open");
	}
	int outft; 
	if((outft = open(data->dest_dir, O_CREAT | O_APPEND | O_WRONLY, 0666)) == -1) {
		perror("open");
	}

	// Use mmap function call to map file to memory
	char* map = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, inft, 0);
	if(map == MAP_FAILED) {
		close(inft);
		perror("map");
		exit(EXIT_FAILURE);
	}

	// Get resource usage
	getrusage(RUSAGE_SELF, &usage);
	usage_time = usage.ru_stime.tv_usec;

	//printf("Page file usage: %ld\n", usage.ru_maxrss);

	// Write memory mapped file to output file
	write(outft, map, size);

	// Unmap file from memory
	if(munmap(map, size) == -1) {
		close(inft);
		perror("munmap");
		exit(EXIT_FAILURE);
	}

	// Close input/output file descriptors
	close(inft);
	close(outft);
	
	// Remove old file
	remove(data->filename);

	// Print cpu time from rusage
	printf("Time: %d [sec]\n", usage_time);

	// Get total bytes transferred for main code
	data->bytes = sizeof(char)*size;

	// Exit thread
	pthread_exit(NULL);

}

long getfilesize(char* filename) {
	FILE *fp;
	long size;
	if((fp = fopen(filename, "r")) == NULL) {
		perror("open");
		return 0;
	}
	fseek(fp, 0L, SEEK_END);
	size = ftell(fp);
	fclose(fp);
	return size;
}

