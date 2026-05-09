#include <stdio.h>
#include "preprocessor.h"
#include "first_pass.h"
#include "second_pass.h"

extern int error_in_pass1;

/*
 * Entry point of the assembler program.
 *
 * Usage:
 *   assembler file1.as
 *
 * For each provided source file:
 *   1. Runs the macro preprocessor to produce .am
 *   2. Executes the first pass (builds symbol table, counts instructions)
 *   3. Executes the second pass (encodes instructions, generates output files)
 */
int main(int argc, char *argv[]) {
int i;

	/* Check if at least one source file is provided*/
	if (argc < 2) {
        	printf("Usage: %s file1.as [file2.as ...] \n", argv[0]);
        	return 1;
	}

	/* Process each input file*/
	for (i = 1; i < argc; i++) {
		printf("Processing file: %s\n", argv[i]);
	run_preprocessor(argv[i]);   /* Step 1: expand macros .am*/
        run_first_pass(argv[i]);     /* Step 2: first pass (symbols, addresses)*/
		if(error_in_pass1){
printf("First pass aborted due to errors. No output files will be created\n");
		}else{
        run_second_pass(argv[i]);    /* Step 3: second pass (encoding, output)*/
		}
	}

return 0;
}
