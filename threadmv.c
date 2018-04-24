// Landon Haugh
// lrh282
// lrh282_threadmv.c
// This program moves multiple files in respective threads
// References: 
// 1. http://pubs.opengroup.org/onlinepubs/009696699/functions/read.html
// 2. http://codewiki.wikidot.com/c:system-calls:write
// 3. https://www.linuxquestions.org/questions/programming-9/mmap-tutorial-c-c-511265/
// 4. Any references from the file search assignment
//
// I have provided a Makefile that compiles both the library and this file for easy building
// The Makefile also compiles this file with a -g flag for gdb/lldb debugging

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
#include <sys/time.h>
#include <sys/resource.h>
#include "libmv.h"

// Global Variables
double total;

// Main function
int main(int argc, char** argv) {
   
    // Arguments check
    if(argc < 3) {
        printf("Invalid arguments\n");
        printf("Usage:\n");
        printf("./threadmv <-b (block size)|-mem> <file(1)> <file(2)> <file(3)> .. <file(n)> <dir>\n");
        exit(-1);
    }
    
	// Array of threads for amount of files to move + directory
    pthread_t threads[argc - 2];
    struct thread_data data[argc - 2];

	// Flag handling
	int mmap_flag = 0;
	int flag = 2;
	if(strcmp(argv[1], "-mem") == 0) {
		printf("-mem found!\n");
		mmap_flag = 1;
		flag = 3;
	} else if(strcmp(argv[1], "-b") == 0) {
		printf("-b found!\n");
		flag = 4;
	}

    for(int t = 0; t < argc - flag; t++) {

		// Flag handling
		if(mmap_flag == 1) {
			strcpy(data[t].filename, argv[t + 2]);
		} else if(flag == 4) {
			strcpy(data[t].filename, argv[t + 3]);
		} else {
			strcpy(data[t].filename, argv[t + 1]);
		}

		// Assign thread number
        data[t].thread_no = t;

		// Assign blocksize
		if(flag == 4) {
			data[t].blocksize = atoi(argv[2]);
		} else {
			data[t].blocksize = -1;
		}

		// Assign destination directory
        strcpy(data[t].dest_dir, argv[argc - 1]);


        // Foung these attribute funcs online, don't know what they do
        pthread_attr_t attr;
        pthread_attr_init(&attr);

        // Create new thread with library function - use mmap if flag is set
        if(mmap_flag == 1) {
			pthread_create(&threads[t], &attr, &mmap_move, &data[t]);
		} else {
			pthread_create(&threads[t], &attr, &movefile, &data[t]);
		}
	}

    // wait for each thread to finish
    for(int t = 0; t < argc - flag; t++) {
        pthread_join(threads[t], NULL);
    }

    // Add up total data bytes transferred
    for(int t = 0; t < argc - flag; t++) {
        total += data[t].bytes;
    }

   	printf("%f total bytes transferred\n", total); 

	return 0;
}
