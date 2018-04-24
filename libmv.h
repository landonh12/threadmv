// Landon Haugh
// lrh282
// libmv.h
//
// Header file for libmv library

#ifndef _LIB_MV_H_
#define _LIB_MV_H_

	// Structure for thread data to be passed as pointer to thread function
	struct thread_data {
   	 	int thread_no;
   	 	int bytes;
		int mmap_flag;
		int blocksize;
   		char dest_dir[128];
   	 	char filename[128];
	};

	// Function prototypes
	extern void* movefile(void* ptr);
	extern void* mmap_move(void* ptr);
	extern long getfilesize(char* filename);

#endif
