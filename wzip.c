#include <stdio.h>
#include "wzip.h"

int prev_char;
int count;

int main(int argc, char *argv[]) {
   if(argc <= 1) {    //if no file after ./wzip
        printf("wzip: file1 [file2 ...]\n");
        return 1;
    }

    for(int i = 1; i < argc; i++) {  //loop through arguments
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



