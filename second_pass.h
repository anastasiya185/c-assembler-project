#ifndef SECOND_PASS_H
#define SECOND_PASS_H

/*
 * Performs the second pass of the assembler on a given file.
 * Generates .ob, .ent and .ext output files.
 */
void run_second_pass(const char *filename);

/* Writes the .ent file with all entry labels and their addresses.*/
void write_ent_file(const char *filename);

/*
 * Writes the .ext file with all external symbol usages.
 * Each line: <label> <address in base-4>
 */
void write_ext_file(const char *filename);
	
#endif
