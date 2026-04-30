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
			if(i == 1) {
				prev_char = data[0];
				count = 1;
				for(int j = 1; j < st.st_size; j++) {
					int new_char = data[j];
					if(new_char == prev_char) {
						count++;
					} else {
						fwrite(&count, sizeof(int), 1, stdout);
                		fwrite(&prev_char, sizeof(char), 1, stdout);
                		prev_char = new_char;
                		count = 1;
            		}
        		}
    		} else {
        		for (int j = 0; j < st.st_size; j++) {
            		int new_char = data[j];

            		if (new_char == prev_char) {
                		count++;
            		} else {
                		fwrite(&count, sizeof(int), 1, stdout);
                		fwrite(&prev_char, sizeof(char), 1, stdout);
                		prev_char = new_char;
                		count = 1;
            		}
        		}
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



