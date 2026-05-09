#ifndef FIRST_PASS_H
#define FIRST_PASS_H
extern int error_in_pass1; 
/*
 * Executes the first pass on a given file.
 * - Handles symbol definitions
 * - Processes .data/.string/.mat/.entry/.extern directives
 * - Stores instruction metadata in memory for the second pass
 */
void run_first_pass(const char *filename);
	
#endif
