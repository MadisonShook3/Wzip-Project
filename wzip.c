#include <stdio.h>
#include "wzip.h"


int main(int argc, char *argv[]) {
	if(argc <= 1) {
		printf("input a file name");
		return 1;
	}

	FILE *file = fopen(argv[1], "r");

	if(file == NULL) {
		printf("Error opening file");
		return 1;
	}

	parse_input(file);
	
	fclose(file);
	
	return 0;
}

void parse_input(FILE *file) {
	int prev_char = fgetc(file);
	if(prev_char == EOF) return;
	int count = 1;

	int new_char;
	while((new_char = fgetc(file)) != EOF) {
		if(new_char == prev_char) {
			count = count + 1;
		} else {
			fwrite(&count, sizeof(int), 1, stdout);
			fwrite(&prev_char, sizeof(char), 1, stdout);
			prev_char = new_char;
			count = 1;
			continue;
		}
	}

	fwrite(&count, sizeof(int), 1, stdout);
    fwrite(&prev_char, sizeof(char), 1, stdout);
}




