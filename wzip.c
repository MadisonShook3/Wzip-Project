#include <stdio.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include "wzip.h"

int prev_char;
int count;

typedef struct {
	char *data;
	int start;
	int end;
	
	int *counts;
	char *chars;
	int size;
} thread_arg_t;

void *worker(void *arg) {
	thread_arg_t *targ = (thread_arg_t *)arg;
	//later compress targ -> data from targ->start to targ->end
	if(targ->start >= targ->end) {
		targ->size = 0;
		return NULL;
	}

	int prev = targ->data[targ->start];
    int count = 1;
    targ->size = 0;

    for(int i = targ->start + 1; i < targ->end; i++) {
        int curr = targ->data[i];

        if(curr == prev) {
            count++;
        } else {
            targ->counts[targ->size] = count;
            targ->chars[targ->size] = prev;
            targ->size++;

            prev = curr;
            count = 1;
        }
    }

    targ->counts[targ->size] = count;
    targ->chars[targ->size] = prev;
    targ->size++;

	return NULL;
}

int main(int argc, char *argv[]) {
   if(argc <= 1) {    //if no file after ./wzip
        printf("wzip: file1 [file2 ...]\n");
        return 1;
    }

    for(int i = 1; i < argc; i++) {  //loop through arguments
		struct stat st;
		stat(argv[i], &st);   //get each file's size
		
		if(st.st_size <= 4096) {  //st.st_size is the file size in bytes I learned this from asking ChatGPT 
			FILE *file = fopen(argv[i], "r"); //open file

        	if(file == NULL) {
            	printf("Error opening file");
            	return 1;
        	} else if(i == 1) {    //if just started on first file,
                               //need to give the global vars values
            	setup_file(file);
        	}
        	parse_input(file);
        	fclose(file);	
		} else {
			//use other code
			int fd = open(argv[i], O_RDONLY);
			if(fd < 0) {
				printf("Error opening file");
				return 1;
			}
			char *data = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
			if(data == MAP_FAILED) {
				printf("mmap failed");
				close(fd);
				return 1;
			}

			int first_start = 0;
			if(i == 1) {
				prev_char = data[0];
				count = 1;
				first_start = 1;
			}

    		pthread_t threads[3];
			thread_arg_t args[3];

			int chunk = st.st_size / 3;

			args[0].data = data;
			args[0].start = first_start;
			args[0].end = chunk;

			args[1].data = data;
			args[1].start = chunk;
			args[1].end = chunk * 2;

			args[2].data = data;
			args[2].start = chunk * 2;
			args[2].end = st.st_size;
			
			for(int t = 0; t < 3; t++) {
				int max_runs = args[t].end - args[t].start;
				args[t].counts = malloc(sizeof(int) * max_runs);
				args[t].chars = malloc(sizeof(char) * max_runs);
				args[t].size = 0;
			}
			// create threads
			for (int t = 0; t < 3; t++) {
    			pthread_create(&threads[t], NULL, worker, &args[t]);
			}

			// join threads
			for (int t = 0; t < 3; t++) {
    			pthread_join(threads[t], NULL);
			}

			for(int t = 0; t < 3; t++) {
    			for(int k = 0; k < args[t].size; k++) {
        			int new_count = args[t].counts[k];
        			char new_char = args[t].chars[k];

        			if(new_char == prev_char) {
            			count += new_count;
        			} else {
            			fwrite(&count, sizeof(int), 1, stdout);
            			fwrite(&prev_char, sizeof(char), 1, stdout);

            			prev_char = new_char;
            			count = new_count;
        			}
    			}
			}
			
			for(int t = 0; t < 3; t++) {
				free(args[t].counts);
				free(args[t].chars);
			}

			munmap(data, st.st_size);
			close(fd);
		}
	}

    fwrite(&count, sizeof(int), 1, stdout);    //write the value
    fwrite(&prev_char, sizeof(char), 1, stdout);   
    //write the character after the value
    
    return 0;
}

void parse_input(FILE *file) {
    int new_char;
    while((new_char = fgetc(file)) != EOF) {
        if(new_char == prev_char) {  
        //if chars are the same, keep count
            count = count + 1;
        } else {
            fwrite(&count, sizeof(int), 1, stdout);
            fwrite(&prev_char, sizeof(char), 1, stdout);
            prev_char = new_char;
            count = 1;
            continue;
        }
    }
}

//just gives the global variables their starting 
//values and if its the end of file, return.
void setup_file(FILE *file) {
    prev_char = fgetc(file);
    if(prev_char == EOF) return;
    count = 1;
}



